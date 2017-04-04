#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int main(int argc, char *argv[]){

    int pgsize, tlbentries, quantum, physpages;

    // check size of input arguments
    // tvm379 pgsize tlbentries { g | p } quantum physpages { f | l } trace1 trace2 ...
    if (argc < 8){
        fprintf(stderr, "Usage: %s pgsize tlbentries { g | p } "
        "quantum physpages { f | l } trace1 trace2 ...\n", argv[0]);
        return(1);
    }

    // verify pgsize
    pgsize = atoi(argv[1]);
    if (!pgsize) {
        fprintf(stderr, "Invalid pgsize\n");
        return(1);
    } else if (pgsize < 16 || pgsize > 65536 || (pgsize & (pgsize - 1)) != 0) {
        fprintf(stderr, "pgsize should always a power of 2 from 16 to 65536\n");
        return(1);
    }

    // verify tlbentries
    tlbentries = atoi(argv[2]);
    if (!tlbentries) {
        fprintf(stderr, "Invalid tlbentries\n");
        return(1);
    } else if (tlbentries < 8 || tlbentries > 256 || (tlbentries & (tlbentries - 1)) != 0) {
        fprintf(stderr, "tlbentries should always a power of 2 from 8 to 256\n");
        return(1);
    }

    // verify quantum
    quantum = atoi(argv[4]);
    if (!quantum) {
        fprintf(stderr, "Invalid quantum\n");
        return(1);
    }

    // verify physpages
    physpages = atoi(argv[5]);
    if (!physpages) {
        fprintf(stderr, "Invalid physpages\n");
        return(1);
    } else if (physpages < 1 || physpages > 10000000) {
        fprintf(stderr, "physpages should in range of (1, 10^7)\n");
        return(1);
    }

    // whether the TLB is uniform across processes (Global/process specific)
    if (strcmp(argv[3], "g")!=0 && strcmp(argv[3], "p")!=0) {
        fprintf(stderr, "The 4th argument should be { g | p }\n");
        return(1);
    }

    // check page eviction policy (FIFO/LRU)
    if (strcmp(argv[6], "f")!=0 && strcmp(argv[6], "l")!=0) {
        fprintf(stderr, "The 7th argument should be { f | l }\n");
        return(1);
    }









}
