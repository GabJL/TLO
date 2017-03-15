#include <iostream>
#include <fstream>
#include <cstdlib>
#include <vector>
#include <string>
#include <ctime> 
#include <algorithm>
#include "cInstance.hpp"
#include "simpleXMLParser.hpp"

using namespace std;

typedef vector<unsigned> Solution;
typedef vector<Solution> Population;

string command;

// Algorithm configuration
const unsigned POP_SIZE = 10;

// If all the TL is yellow is a phase for pedestrian (4*lanes)
bool areAllYellow(string phase) {
  for (int i = 0; i < phase.size(); i++)
    if (phase[i] != 'y')
      return false;
  return true;
}

void generateSolution(Solution &solution, const cInstance &c)
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

void writeSolutionFile(const Solution &solution)
{
	ofstream f("tl.txt");
	for(int i = 0; i < solution.size(); i++)
		f <<  solution[i] << " ";
	f.close();
}

void readFitnessFile(float &fitness)
{
	ifstream f("result.txt");
	string s;
	for(int i = 0; i < 6; i++) // skip lines
		getline(f,s);
	f >> fitness;
	f.close();
}

float evaluateSolution(const Solution &solution){
	float fitness;
	writeSolutionFile(solution);
	system(command.c_str());
	readFitnessFile(fitness);
	return fitness;
}

void initializePopulation(Population &pop, vector<float> &fitness, const cInstance &c){
	fitness.clear();
	for(int i = 0; i < pop.size(); i++){
		generateSolution(pop[i],c);
		fitness.push_back(evaluateSolution(pop[i]));
	}
}

void selectParents(const Population &p, Solution &i1, Solution &i2){
	int pos1 = rand()%(p.size()-1) + 1;
	if(rand()*1.0/RAND_MAX  > 0.5){
		i1 = p[pos1];
		i2 = p[rand()%pos1];
	} else {
		i1 = p[rand()%pos1];
		i2 = p[pos1];
	}
}

void recombine(Solution &i1,const Solution &i2){
	int pos = rand()%(i1.size() - 1);
	for(int i = pos; i < i1.size(); i++){
		i1[i] = i2[i];
	}
}

void mutate(Solution &s, const cInstance &c){
	bool changed;
	do{
		vector<string> phases;
		int pos = rand()%(s.size());
		int counter = 0;
		changed = false;
		for(int j = 0; j < c.getNumberOfTLlogics(); j++)
		{
			phases = c.getPhases(j);
			if(counter + phases.size() > pos) break;
			counter += phases.size();
		}
		if(!areAllYellow(phases[pos-counter]) && !isSubString(phases[pos-counter],"y",pos)){
			s[pos] = rand()%55 + 5;
			changed = true;
		}
	}while(!changed);
}

void merge(Population &p, const  Solution &s, vector<float> &fitness){
	float fit = evaluateSolution(s);
	int pos = rand()%p.size();
	if(fitness[pos] > fit){
		fitness[pos] = fit;
		p[pos] = s;
	}
}

int main(int argc, char **argv)
{
	Population population(POP_SIZE);
	vector<float> fitness;
	Solution ind1, ind2;
	unsigned steps;
	cInstance c;

	srand(time(0));

	if(argc < 3)
	{
		cout << "Usage: " << argv[0] << " <instance> <number of generations>" << endl;
		exit(-1);
	}

	c.read(argv[1]);
	steps = atoi(argv[2]);
	command = "./sumo-wrapper " + string(argv[1]) + " " + "tl.txt result.txt";

	initializePopulation(population,fitness, c);
	// REMOVE!!!
	int pos = distance(fitness.begin(), min_element(fitness.begin(), fitness.end()));
	cout << fitness[pos] << endl;

	for(int i = POP_SIZE; i < steps; i++)
	{
		selectParents(population, ind1, ind2);
		recombine(ind1, ind2);
		mutate(ind1,c);
		merge(population,ind1,fitness);
		// REMOVE!!!
		pos = distance(fitness.begin(), min_element(fitness.begin(), fitness.end()));
		cout << fitness[pos] << endl;
	}

	pos = distance(fitness.begin(), min_element(fitness.begin(), fitness.end()));
	cout << "Best solution: " << endl;
	for(int i = 0; i < population[pos].size(); i++)
		cout << population[pos][i] << " ";
	cout << endl << "Fitness: " << fitness[pos] << endl;

	return 0;
}



