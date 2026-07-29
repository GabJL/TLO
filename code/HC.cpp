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

// tl.txt/result.txt names, PID-prefixed so several instances of this
// algorithm (or of other algorithms) can run concurrently in the same
// directory without clobbering each other
string tlFile, resultFile;

// Aborts if the last system() call (running sumo-wrapper) did not succeed
void checkExecution(int ret)
{
	if(ret == -1 || !WIFEXITED(ret) || WEXITSTATUS(ret) != 0)
	{
		cerr << "Error: sumo-wrapper execution failed" << endl;
		exit(-1);
	}
}

// If all the TL is yellow is a phase for pedestrian (4*lanes)
bool areAllYellow(string phase) {
  for (int i = 0; i < phase.size(); i++)
    if (phase[i] != 'y')
      return false;
  return true;
}

void generateSolution(vector<unsigned> &solution, const cInstance &c)
{
	solution.clear();

	// Random Values
/*	
	for(int i = 0; i < c.getTotalNumberOfPhases(); i++)
	{
		solution.push_back(rand()%56 + 4);
	}
*/
	// "Advanced" solution
	vector<string> phases;
	int pos;
	for(int j = 0; j < c.getNumberOfTLlogics(); j++)
	{
		phases = c.getPhases(j);
		for(int k = 0; k < phases.size(); k++)
		{
			if(areAllYellow(phases[k]))
				solution.push_back(4*phases[k].size());		
			else if(isSubString(phases[k],"y",pos))
				solution.push_back(4);		
			else
				solution.push_back(rand()%55 + 5);
		}
	}
}

void writeSolutionFile(const vector<unsigned> &solution, const cInstance &c)
{
	ofstream f(tlFile.c_str());
	for(int i = 0; i < c.getTotalNumberOfPhases(); i++)
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

void generateNeighbor(const vector<unsigned> &sol, vector<unsigned> &neig){
	int pos = rand()%sol.size(); 	// Posición a cambiar
	neig = sol;		// el vecino es igual al actual 
	neig[pos] = rand()%56 + 5;	// Ponemos un valor aleatoria en esa posición
}

int main(int argc, char **argv)
{
	vector<unsigned> solution, neigh;
	float fitness, fitness_neigh;
	unsigned steps;
	string command;
	cInstance c;

	srand(time(0) ^ getpid()); // XOR with the PID: time(0) alone has 1s resolution and would give
	                           // the same sequence to processes launched in the same second

	if(argc < 3)
	{
		cout << "Usage: " << argv[0] << " <instance> <number of iterations>" << endl;
		exit(-1);
	}

	c.read(argv[1]);

	steps = atoi(argv[2]);

	tlFile = "tl_" + to_string(getpid()) + ".txt";
	resultFile = "result_" + to_string(getpid()) + ".txt";
	command = "./sumo-wrapper " + string(argv[1]) + " " + tlFile + " " + resultFile;

	generateSolution(solution, c);
	writeSolutionFile(solution, c);
	checkExecution(system(command.c_str()));
	readFitnessFile(fitness);
	cout << fitness << endl;

	for(int i = 1; i < steps; i++)
	{
		generateNeighbor(solution, neigh);		
		writeSolutionFile(neigh, c);
		checkExecution(system(command.c_str()));
		readFitnessFile(fitness_neigh);
		if(fitness_neigh < fitness)
		{
			solution = neigh; 
			fitness = fitness_neigh;
		}
		cout << fitness << endl;
	}

	cout << "Best solution: " << endl;
	for(int i = 0; i < c.getTotalNumberOfPhases(); i++)
		cout << solution[i] << " ";
	cout << endl << "Fitness: " << fitness << endl;

	remove(tlFile.c_str());
	remove(resultFile.c_str());

	return 0;
}



