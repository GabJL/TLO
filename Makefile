all:
	(cd code; make all)

clean:
	(rm -f tl.txt result.txt tl_*.txt result_*.txt HC ssGA sumo-wrapper RS PSO)
	(cd code; make clean)
	(cd instances; make clean)
