all: sumo-wrapper genInstanceFile RS getOriginalTL ssGA HC
	cp sumo-wrapper RS ssGA HC ..

clean:
	rm sumo-wrapper genInstanceFile RS getOriginalTL ssGA HC

sumo-wrapper: sumo-wrapper.cpp cInstance.cpp simpleXMLParser.cpp
	g++ -o sumo-wrapper sumo-wrapper.cpp cInstance.cpp simpleXMLParser.cpp

genInstanceFile: genInstanceFile.cpp simpleXMLParser.cpp
	g++ -o genInstanceFile genInstanceFile.cpp simpleXMLParser.cpp

RS: RS.cpp simpleXMLParser.cpp cInstance.cpp
	g++ -o RS RS.cpp simpleXMLParser.cpp cInstance.cpp

HC: HC.cpp simpleXMLParser.cpp cInstance.cpp
	g++ -o HC HC.cpp simpleXMLParser.cpp cInstance.cpp

ssGA: ssGA.cpp simpleXMLParser.cpp cInstance.cpp
	g++ -o ssGA ssGA.cpp simpleXMLParser.cpp cInstance.cpp

getOriginalTL: getOriginalTL.cpp simpleXMLParser.cpp
	g++ -o getOriginalTL getOriginalTL.cpp simpleXMLParser.cpp

