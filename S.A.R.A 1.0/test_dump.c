#include <stdio.h>

int main() {
    #ifdef WIN64
        printf("WIN64 is defined\n");
    #else
        printf("WIN64 is not defined\n");
    #endif

    return 0;
}

