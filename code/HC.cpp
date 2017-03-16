#include <iostream>
#include <fstream>
#include <cstdlib>
#include <vector>
#include <string>
#include <ctime> 
#include "cInstance.hpp"
#include "simpleXMLParser.hpp"

using namespace std;

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
	ofstream f("tl.txt");
	for(int i = 0; i < c.getTotalNumberOfPhases(); i++)
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

	srand(time(0));

	if(argc < 3)
	{
		cout << "Usage: " << argv[0] << " <instance> <number of iterations>" << endl;
		exit(-1);
	}

	c.read(argv[1]);

	steps = atoi(argv[2]);

	command = "./sumo-wrapper " + string(argv[1]) + " " + "tl.txt result.txt";

	generateSolution(solution, c);
	writeSolutionFile(solution, c);
	system(command.c_str());
	readFitnessFile(fitness);
	cout << fitness << endl;

	for(int i = 1; i < steps; i++)
	{
		generateNeighbor(solution, neigh);		
		writeSolutionFile(neigh, c);
		system(command.c_str());
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
	cout << endl << "Fitness: " << fitness_neigh << endl;

	return 0;
}



