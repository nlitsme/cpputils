#include <boost/format.hpp>
#include <iostream>

int main(int argc, char* argv[])
{
    const long maxIter = 2000000L;
    int total = 0;
    for(long i = 0; i < maxIter; ++i) {
        auto f = boost::format("%0.10f:%04d:%+g:%s:%p:%c:%%\n") %
                1.234 % 42 % 3.13 % "str" % (void*)1000 % (char)'X';
        total += f.str().size();
    }
    std::cout << "total = " << total << "\n";
    return 0;
}

