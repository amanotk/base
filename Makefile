# -*- Makefile -*-

# include path
INCLUDE_PATH=$(HOME)/usr/include

# compilers
CXX      = g++
CXXFLAGS = -O3 -I$(INCLUDE_PATH)

%.o : %.cpp
	$(CXX) -c $(CXXFLAGS) $<

default: TestConfig TestNArray TestSArray TestMersenneTwister

TestConfig: TestConfig.o
	$(CXX) $(CXXFLAGS) $< -o $@

TestNArray: TestNArray.o
	$(CXX) $(CXXFLAGS) $< -o $@

TestSArray: TestSArray.o
	$(CXX) $(CXXFLAGS) $< -o $@

TestMersenneTwister: TestMersenneTwister.o
	$(CXX) $(CXXFLAGS) $< -o $@

clean:
	rm -f *.o *.out

cleanexe:
	rm -f TestConfig TestNArray TestSArray TestMersenneTwister

