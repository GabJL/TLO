REQUIREMENTS:
-------------
1) C++ compiler
2) SUMO simulator (SUMO executable should be in the PATH variable)
3) network and route files for SUMO with the following format: Y/X.net.xml and Y/X.rou.xml

PROVIDED TOOL:
--------------
1) sumo-wrapper: executes SUMO on a concrete instance using a provided TL configuration and gets statistical values about the execution
2) genInstanceFile: builds the instance file used by sumo-wrapper and maybe your algorithm
3) RS: random search
4) getOriginalTL: obtains the original TL configuration from network file

INSTANCE FILE CONTENT:
----------------------
Line   1: Instance name
Line   2: Path
Line   3: Number of tlLogics 
Line   4: totalPhases 
Line 5-N: tlID #phase phases 
Line N+1: numVehicles 
Line N+2: simTime

HOW TO GENERATE A NEW INSTANCE FILE:
------------------------------------
1) Execute genInstance Y X T where Y is the path, X is the instance name, and T is the simulation time

HOW TO INTEGRATE SUMO IN YOUR ALGORITHM
---------------------------------------
1) Your algorithm should build a file with "totalPhases" number (maybe it should read the instance file)
2) execute: sumo-wrapper <instanceFile> <yourTLConfiguration> <resultsFile>
3) Read <resultsFile>

RESULT FILE CONTENT:
--------------------
float number       // Original Green vs Red
float number       // Normalized GvR
unsigned number    // Total duration
unsigned number	   // Vehicles arriving
unsigned number    // Vehicles not arriving
unsigned number    // Number of stops
float number       // Fitness
float number       // Mean Travel Time
float number       // Mean Waiting Time
float number       // CO2 (mg/s)
float number       // CO (mg/s)
float number       // HC (mg/s)
float number       // NOx (mg/s)
float number       // PMx (mg/s)
float number       // fuel (ml/s)
float number       // noise (dB)

EXAMPLES:
---------

1) Generating malaga instance File (requires instances/malaga/malaga.net.xml and instances/malaga/malaga.rou.xml):
1.a) ./genInstanceFile instances/malaga/ malaga 2200
1.b) mv malaga.txt instanceFiles/
2) Running SUMO using an existing TL configuration (tl.txt) on malaga instance
2.a) ./sumo-wrapper instanceFiles/malaga.txt tl.txt result.txt
2.b) cat result.txt
3) Running RS (10 steps) on malaga instance
3.a) ./RS instanceFiles/malaga.txt 10
4) Getting the original TL configuration of malaga instance
4.a) ./getOriginalTL instances/malaga/malaga.net.xml
