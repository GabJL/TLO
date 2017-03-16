all:
	(cd code; make all)

clean:
	(rm tl.txt result.txt HC ssGA sumo-wrapper RS)
	(cd code; make clean)
	(cd instances; make clean)
