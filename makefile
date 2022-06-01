filefinder: libff.a
	g++ -std=c++17 findfilelib.hpp main.cpp libff.a

libff.a:
	g++ -c -std=c++17 findfilelib.cpp -o libff.o
	ar rcs libff.a libff.o
	ranlib libff.a