CXXFLAGS=-g $(if $(D),-O0,-O3) -std=c++1z -Wall
CXXFLAGS+=-I .
LDFLAGS=-g
CDEFS?=-DWITH_CATCH
all: unittests

unittests: unittests.o unittests2.o test-asn1.o
	$(CXX) $(LDFLAGS) -o $@  $^

%.o: tests/%.cpp
	$(CXX) $(CXXFLAGS) $(CDEFS) -c $^ -o $@


clean:
	$(RM) unittests $(wildcard *.o)
