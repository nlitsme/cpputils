#include <stdio.h>

int main(int argc, char* argv[])
{
    const long maxIter = 2000000L;
    char buf[256];
    int total = 0;
    for(long i = 0; i < maxIter; ++i) {
        total += sprintf(buf, "%0.10f:%04d:%+g:%s:%p:%c:%%\n",
                1.234, 42, 3.13, "str", (void*)1000, (int)'X');
    }
    printf("total: %d\n", total);
    return 0;
}


