#include <iostream>
#include <fstream>
#include <cstdlib>
#include <vector>
#include <string>
#include <ctime>
#include <sys/wait.h>
#include <unistd.h>
#include <cstdio>
#include "cInstance.hpp"
#include "simpleXMLParser.hpp"

using namespace std;

typedef vector<unsigned> Solution;
typedef vector<double> RealVector;

string command;
// tl.txt/result.txt names, PID-prefixed so several instances of this
// algorithm (or of other algorithms) can run concurrently in the same
// directory without clobbering each other
string tlFile, resultFile;

// Algorithm configuration
const unsigned SWARM_SIZE = 10;
const double W  = 0.7;	// Inertia weight
const double C1 = 1.5;	// Cognitive coefficient (pull towards pbest)
const double C2 = 1.5;	// Social coefficient (pull towards gbest)

// Bounds for the "movable" phases (mirrors RS/HC/ssGA's rand()%55 + 5)
const double MIN_DURATION = 5;
const double MAX_DURATION = 59;
const double VMAX = MAX_DURATION - MIN_DURATION; // velocity clamping

// If all the TL is yellow is a phase for pedestrian (4*lanes)
bool areAllYellow(string phase) {
  for (int i = 0; i < phase.size(); i++)
    if (phase[i] != 'y')
      return false;
  return true;
}

// A dimension is "fixed" (never searched) when it is a pedestrian phase
// (all yellow) or a transition phase (contains yellow): RS/HC/ssGA never
// touch those either, they always get the same value.
void classifyDimensions(const cInstance &c, vector<bool> &isMutable, vector<unsigned> &fixedValue)
{
	isMutable.clear();
	fixedValue.clear();

	vector<string> phases;
	int pos;
	for(int j = 0; j < c.getNumberOfTLlogics(); j++)
	{
		phases = c.getPhases(j);
		for(int k = 0; k < phases.size(); k++)
		{
			if(areAllYellow(phases[k]))
			{
				isMutable.push_back(false);
				fixedValue.push_back(4*phases[k].size());
			}
			else if(isSubString(phases[k],"y",pos))
			{
				isMutable.push_back(false);
				fixedValue.push_back(4);
			}
			else
			{
				isMutable.push_back(true);
				fixedValue.push_back(0);
			}
		}
	}
}

// Rounds and (thanks to the boundary handling in the main loop) clamps the
// continuous position into a valid discrete TL configuration
void buildSolution(const RealVector &x, const vector<bool> &isMutable, const vector<unsigned> &fixedValue, Solution &sol)
{
	sol.resize(x.size());
	for(int d = 0; d < x.size(); d++)
		sol[d] = isMutable[d] ? (unsigned)(x[d] + 0.5) : fixedValue[d];
}

void writeSolutionFile(const Solution &solution)
{
	ofstream f(tlFile.c_str());
	for(int i = 0; i < solution.size(); i++)
		f <<  solution[i] << " ";
	f.close();
}

void readFitnessFile(float &fitness)
{
	ifstream f(resultFile.c_str());
	string s;
	for(int i = 0; i < 6; i++) // skip lines
		getline(f,s);
	f >> fitness;
	f.close();
}

// Aborts if the last system() call (running sumo-wrapper) did not succeed
void checkExecution(int ret)
{
	if(ret == -1 || !WIFEXITED(ret) || WEXITSTATUS(ret) != 0)
	{
		cerr << "Error: sumo-wrapper execution failed" << endl;
		remove(tlFile.c_str());
		remove(resultFile.c_str());
		exit(-1);
	}
}

float evaluateSolution(const Solution &solution){
	float fitness;
	writeSolutionFile(solution);
	checkExecution(system(command.c_str()));
	readFitnessFile(fitness);
	return fitness;
}

// Random value in [-1,1]
double randUnit(){
	return rand()*2.0/RAND_MAX - 1.0;
}

int main(int argc, char **argv)
{
	vector<bool> isMutable;
	vector<unsigned> fixedValue;
	vector<RealVector> x(SWARM_SIZE), v(SWARM_SIZE), pbest_x(SWARM_SIZE);
	vector<float> fitness(SWARM_SIZE), pbest_fit(SWARM_SIZE);
	Solution sol, gbest_sol;
	RealVector gbest_x;
	float gbest_fit;
	unsigned steps, evals, dim;
	cInstance c;

	srand(time(0) ^ getpid()); // XOR with the PID: time(0) alone has 1s resolution and would give
	                           // the same sequence to processes launched in the same second

	if(argc < 3)
	{
		cout << "Usage: " << argv[0] << " <instance> <number of evaluations>" << endl;
		exit(-1);
	}

	c.read(argv[1]);
	steps = atoi(argv[2]);
	tlFile = "tl_" + to_string(getpid()) + ".txt";
	resultFile = "result_" + to_string(getpid()) + ".txt";
	command = "./sumo-wrapper " + string(argv[1]) + " " + tlFile + " " + resultFile;

	classifyDimensions(c, isMutable, fixedValue);
	dim = isMutable.size();

	// Initialize swarm
	for(int i = 0; i < SWARM_SIZE; i++)
	{
		x[i].resize(dim);
		v[i].resize(dim);
		for(int d = 0; d < dim; d++)
		{
			if(isMutable[d])
			{
				x[i][d] = MIN_DURATION + rand()%((unsigned)(MAX_DURATION-MIN_DURATION) + 1);
				v[i][d] = randUnit()*VMAX;
			}
			else
			{
				x[i][d] = fixedValue[d];
				v[i][d] = 0;
			}
		}

		buildSolution(x[i], isMutable, fixedValue, sol);
		fitness[i] = evaluateSolution(sol);
		pbest_x[i] = x[i];
		pbest_fit[i] = fitness[i];

		if(i == 0 || fitness[i] < gbest_fit)
		{
			gbest_fit = fitness[i];
			gbest_x = x[i];
			gbest_sol = sol;
		}
	}
	evals = SWARM_SIZE;
	cout << gbest_fit << endl;

	// Main loop: standard PSO velocity/position update, kept in continuous
	// space, discretized (rounded) only when a solution has to be evaluated
	while(evals < steps)
	{
		for(int i = 0; i < SWARM_SIZE && evals < steps; i++)
		{
			for(int d = 0; d < dim; d++)
			{
				if(!isMutable[d]) continue;

				double r1 = rand()*1.0/RAND_MAX;
				double r2 = rand()*1.0/RAND_MAX;

				v[i][d] = W*v[i][d] + C1*r1*(pbest_x[i][d]-x[i][d]) + C2*r2*(gbest_x[d]-x[i][d]);
				if(v[i][d] > VMAX) v[i][d] = VMAX;
				if(v[i][d] < -VMAX) v[i][d] = -VMAX;

				x[i][d] += v[i][d];
				// Absorbing boundaries: clamp to the valid range and kill the
				// velocity component so the particle doesn't keep pushing
				// against the same bound on every following iteration
				if(x[i][d] < MIN_DURATION) { x[i][d] = MIN_DURATION; v[i][d] = 0; }
				if(x[i][d] > MAX_DURATION) { x[i][d] = MAX_DURATION; v[i][d] = 0; }
			}

			buildSolution(x[i], isMutable, fixedValue, sol);
			fitness[i] = evaluateSolution(sol);
			evals++;

			if(fitness[i] < pbest_fit[i])
			{
				pbest_fit[i] = fitness[i];
				pbest_x[i] = x[i];
			}
			if(fitness[i] < gbest_fit)
			{
				gbest_fit = fitness[i];
				gbest_x = x[i];
				gbest_sol = sol;
			}
			cout << gbest_fit << endl;
		}
	}

	cout << "Best solution: " << endl;
	for(int i = 0; i < gbest_sol.size(); i++)
		cout << gbest_sol[i] << " ";
	cout << endl << "Fitness: " << gbest_fit << endl;

	remove(tlFile.c_str());
	remove(resultFile.c_str());

	return 0;
}
