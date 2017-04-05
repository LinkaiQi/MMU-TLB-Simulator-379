#include <stdio.h>
#include <stdlib.h>

struct tlb_entrie {
    unsigned int pg_num;
    unsigned int fm_num;
    int ASID; // if it is process specific TLB, then ASID is not used
    int valid;
    // variables needs for double-linked-list
    struct tlb_entrie *next = NULL;
    struct tlb_entrie *prev = NULL;
};

struct page_entrie {
    unsigned int pg_num;
    unsigned int fm_num;
    int pid;
    int valid;
};

// if it is global TLB, pid pass as 0, else pid is the actual process id
int lookup_tlb(struct tlb_entrie **head_ptr, struct tlb_entrie **tail_ptr, unsigned int page_num, int pid) {
    // get head and tail
    struct tlb_entrie *head = *head_ptr;
    struct tlb_entrie *tail = *tail_ptr;
    //start from the first link
    struct tlb_entrie* current = head;
    //navigate through TLB table
    while(current->pg_num != page_num || current->ASID != pid) {
        //if it is last entry
        if(current->next == NULL) {
            return 0;
        } else {
            //go to next entry
            current = current->next;
        }
    }
    /* if entry found (TLB hit), return true and
        put the entry into the head of the TLB table (LRU) */
    // First, remove the hit entry from TLB table
    if (current->prev != NULL) {
        current->prev->next = current->next;
    } else {
        head = current->next;
    }
    if (current->next != NULL) {
        current->next->prev = current->prev;
    } else {
        tail = current->prev;
    }
    // then add to the removed entry to the head of TLB table
    current->prev = NULL;
    current->next = head;
    head->prev = current;
    head = current;

    // if the entry in invalid
    if (current->valid == 0) {
        current->valid = 1;
        return 2;
    }
    return 1;
}

int update_tlb(struct tlb_entrie **head_ptr, struct tlb_entrie **tail_ptr, unsigned int page_num, int pid) {
    struct tlb_entrie *tail = *tail_ptr;
    struct tlb_entrie *head = *head_ptr;
    // evict the least recently used entry
    struct tlb_entrie *temp = tail->perv;
    temp->Next = NULL;
    free(tail);
    tail = temp;
    // update TLB (add new entry)
    struct tlb_entrie *new_entry = (struct tlb_entrie*) malloc(sizeof(struct tlb_entrie));
    head->prev = new_entry
    new_entry->pg_num = page_num;
    new_entry->fm_num = 0;
    new_entry->ASID = pid;
    new_entry->valid = 1;
    new_entry->next = head;
    new_entry->prev = NULL;
    head = new_entry;
}

int get_N_offset(int pgsize) {
    int n = 0
    while (pgsize != 1) {
        pgsize = pgsize / 2;
        n++;
    }
    return n;
}


int main(int argc, char *argv[]){

    int i,ref;
    int *phys_pg;
    int pgsize, tlbentries, quantum, physpages;
    unsigned int pg_num;
    FILE **processes;

    /* ---------------------- check and convert input argvs --------------------------*/
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
    } else if (tlbentries < 8 || tlbentries > 256 ||
        (tlbentries & (tlbentries - 1)) != 0) {
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
    /* -------------------- end check and convert input argvs ------------------------*/

    // allocate memory to store trace file pointers
    processes = (FILE **) malloc(sizeof(FILE*) * (argc -7));
    for (i = 7; i < argc; i++) {
        processes[i-7] = fopen(argv[i], "rb");
        if (!processes[i-7]) {
            fprintf(stderr, "cannot open file: %s\n", argv[i]);
            return(1);
        }
    }

    // allocate simulated physical memory, and initalize to zero (indicate free memory)
    phys_pg = (int *) calloc(sizeof(int) * physpages);

    // get number of offset bits
    int len_offset = get_N_offset(pgsize);

    // reading memory references from each file in cyclical order
    unsigned int reference;
    for (i = 0; i < argc-7; i++) {
        for (ref = 0; ref < quantum; ref++) {
            if (!read(&reference,4,1,processes[i]);) {
                break;  // EOF
            }
            // right shift len_offset bit (get page number)
            pg_num = reference >> len_offset;
        }
    }











}
