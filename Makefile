CXXFLAGS=-O3 -std=c++1z -Wall
CDEFS?=-DWITH_CATCH
all: unittests

unittests: unittests.o unittests2.o
	$(CXX) $(LDFLAGS) -o $@  $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(CDEFS) -c $^ -o $@


clean:
	$(RM) unittests $(wildcard *.o)
