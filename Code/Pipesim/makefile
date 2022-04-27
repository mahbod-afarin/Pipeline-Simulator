pipesim: pipeline.o main.o
	g++ pipeline.o main.o -o pipesim

pipeline.o: pipeline.cpp
	g++ -c pipeline.cpp

main.o: main.cpp
	g++ -c main.cpp

clean: 
	rm *.o pipesim

