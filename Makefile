CXXFLAGS=-O3 -std=c++1z

all: unittests

unittests: unittests.o utfcvutils.o
	$(CXX) $(LDFLAGS) -o $@  $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS)  -c $^ -o $@

