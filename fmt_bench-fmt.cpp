#include <fmt/printf.h>

int main(int argc, char* argv[])
{
    const long maxIter = 2000000L;
    for(long i = 0; i < maxIter; ++i)
        fmt::printf("%0.10f:%04d:%+g:%s:%p:%c:%%\n",
                1.234, 42, 3.13, "str", (void*)1000, (int)'X');
    return 0;
}

