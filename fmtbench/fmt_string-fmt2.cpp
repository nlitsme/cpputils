#include <fmt/format.h>
#include <iostream>

int main(int argc, char* argv[])
{
    const long maxIter = 2000000L;
    int total = 0;
    for(long i = 0; i < maxIter; ++i) {
        std::string res = fmt::format("{:.10f}:{:04}:{:+g}:{}:{}:{}:%\n",
                1.234, 42, 3.13, "str", (void*)1000, (char)'X');
        total += res.size();
    }
    std::cout << "total = " << total << "\n";
    return 0;
}

