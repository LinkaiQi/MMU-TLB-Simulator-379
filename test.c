#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]){
    printf("%lu\n", sizeof(int));
    printf("%lu\n", sizeof(char));

    long n = 0;

    for (long i = 0; i < 100000000; i++) {
        n = n + i;
    }

    printf("%ld\n", n);
}
