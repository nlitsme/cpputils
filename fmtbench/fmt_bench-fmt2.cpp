#include <fmt/format.h>

int main(int argc, char* argv[])
{
    const long maxIter = 2000000L;
    for(long i = 0; i < maxIter; ++i)
        fmt::print("{:.10f}:{:04}:{:+g}:{}:{}:{}:%\n",
                1.234, 42, 3.13, "str", (void*)1000, (char)'X');
    return 0;
}


