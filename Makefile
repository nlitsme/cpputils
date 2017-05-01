CXXFLAGS=-O3 -std=c++1z -Wall

all: unittests

unittests: unittests.o unittests2.o
	$(CXX) $(LDFLAGS) -o $@  $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS)  -c $^ -o $@

