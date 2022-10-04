CXX = g++
CXXFLAGS = -Wall -g 

mytest: mytest.cpp file.o hash.o
	$(CXX) $(CXXFLAGS) file.o hash.o mytest.cpp -o mytest

hash.o: hash.cpp hash.h
	$(CXX) $(CXXFLAGS) -c hash.cpp

file.o: file.cpp file.h
	$(CXX) $(CXXFLAGS) -c file.cpp

clean:
	rm mytest hash.o file.o

.PHONY: 
	clean

make val:
	valgrind ./mytest
