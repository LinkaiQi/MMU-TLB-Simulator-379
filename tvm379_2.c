#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define GLOBAL 0
#define PROCESS 1

#define FIFO 0
#define LRU 1

// global variables
struct free_fm *free_fm_head = NULL;
struct reference *reference_head = NULL;
struct reference *reference_tail = NULL;
// (Global) TLB
struct tlb_entrie* TLB_head;
struct tlb_entrie* TLB_tail;

struct tlb_entrie {
    unsigned int pg_num;
    unsigned int fm_num;
    int ASID; // if it is process specific TLB, then ASID is not used
    int valid;
    // variables needs for double-linked-list
    struct tlb_entrie *next;
    struct tlb_entrie *prev;
};


// the (page_entrie address - base address) / sizeof(page_entrie) is page number
struct page_entrie {
    // unsigned int pg_num;
    unsigned int fm_num;
    int valid;
};

// physical memory entry
struct phys_entry {
    // unsigned int fm_num;
    // indicate this entry is be mapped into which virtual memory address
    unsigned int pg_num;
    int pid;
    int valid;
};

struct free_fm {
    int fm_num;
    struct free_fm *next;
};

struct reference {
    int fm_num;
    struct reference *next;
    struct reference *prev;
};

// void referenceFrame(int frame, int ev_policy) {
//    struct reference new_reference;
//    new_reference.frame = frame;
//    new_reference.next = reference_head;
//    new_reference.
//    current = reference_head;
//    while (current != NULL) {
//        if (current->fm_num == frame) {
//            /* code */
//        }
//    }
// }

void addFrameReference(int frame) {
    struct reference *new_ref = (struct reference *) malloc(sizeof(struct reference));
    new_ref->fm_num = frame;
    new_ref->prev = reference_head;
    if (reference_head != NULL) {
        reference_head->prev = new_ref;
    } else {
        reference_tail = new_ref;
    }
    reference_head = new_ref;
}

// LRU ONLY
void updateFrameReference(int frame) {
    // search the free frame list, find the referenced frame
    struct reference *current = reference_head;
    while (current != NULL) {
        if (current->fm_num == frame) {
            break;
        }
        current = current->next;
    }
    // update the frame (move the frame to the head) LRU ONLY
    struct reference *prevFrame = current->prev;
    struct reference *nextFrame = current->next;

    // remove the frame
    if (prevFrame != NULL) {
        prevFrame->next = nextFrame;
    } else {
        reference_head = nextFrame;
    }
    if (nextFrame != NULL) {
        nextFrame->prev = prevFrame;
    } else {
        reference_tail = prevFrame;
    }

    // add the frame to the head
    current->prev = NULL;
    current->next = reference_head;
    reference_head->prev = current;
    reference_head = current;
}

int evictFrameReference() {
    // remove the last reference in the list
    struct reference *evited_ref = reference_tail;
    reference_tail = evited_ref->prev;
    reference_tail->next = NULL;

    // add it to the head
    evited_ref->next = reference_head;
    evited_ref->prev = NULL;
    reference_head->prev = evited_ref;
    reference_head = evited_ref;

    return evited_ref->fm_num;
}

void init_TLB_P(int tlbentries, int n_process, struct tlb_entrie** TLB_heads, struct tlb_entrie** TLB_tails) {
    int p, i;
    for (p = 0; p < n_process; p++) {
        // initial tlbentries entries
        // create tail entry
        struct tlb_entrie *entry = (struct tlb_entrie*) calloc(1, sizeof(struct tlb_entrie));
        entry->next = NULL;
        entry->prev = NULL;
        TLB_heads[p] = entry;
        TLB_tails[p] = entry;
        // create rest tlbentries-1 entries
        for (i = 0; i < tlbentries-1; i++) {
            entry = (struct tlb_entrie*) calloc(1, sizeof(struct tlb_entrie));
            entry->next = TLB_heads[p];
            TLB_heads[p]->prev = entry;
            TLB_heads[p] = entry;
        }
    }
}

void init_TLB_G(int tlbentries) {
    int i;
    struct tlb_entrie *entry;
    // create tail entry
    entry = (struct tlb_entrie*) calloc(1, sizeof(struct tlb_entrie));
    entry->next = NULL;
    entry->prev = NULL;
    TLB_head = entry;
    TLB_tail = entry;
    // create rest tlbentries-1 entries
    for (i = 0; i < tlbentries-1; i++) {
        entry = (struct tlb_entrie*) calloc(1, sizeof(struct tlb_entrie));
        entry->next = TLB_head;
        TLB_head->prev = entry;
        TLB_head = entry;
    }
}

// if it is process TLB, pid pass as 0, else pid is the actual process id
int lookup_tlb(struct tlb_entrie **head_ptr, struct tlb_entrie **tail_ptr, unsigned int page_num, int pid) {
    // get head and tail
    struct tlb_entrie *head = *head_ptr;
    struct tlb_entrie *tail = *tail_ptr;
    //start from the first entry
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

    /*
    int i = 1;
    while (current!= NULL) {
        i ++;
        current = current->next;
        printf("lalalala %d\n", i);
    }
    */


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
    if (head->valid == 0) {
        head->valid = 1;
        return 1;
    }
    return 2;
}

void update_tlb(struct tlb_entrie **head_ptr, struct tlb_entrie **tail_ptr, unsigned int page_num, int pid) {
  //printf("call update_tlb\n");
    struct tlb_entrie *tail = *tail_ptr;
    struct tlb_entrie *head = *head_ptr;

    // evict the least recently used entry
    struct tlb_entrie *temp = tail->prev;
    temp->next = NULL;
    //free(tail);
    tail = temp;

    // update TLB (add new entry)
    struct tlb_entrie *new_entry = (struct tlb_entrie*) malloc(sizeof(struct tlb_entrie));
    head->prev = new_entry;
    new_entry->pg_num = page_num;
    new_entry->fm_num = 0;
    new_entry->ASID = pid;
    new_entry->valid = 1;
    new_entry->next = head;
    new_entry->prev = NULL;
    head = new_entry;
}

void setEvictedEntryToInvalid_tlb(struct tlb_entrie **head_ptr, struct tlb_entrie **tail_ptr, unsigned int page_num, int pid) {
    //struct tlb_entrie *tail = *tail_ptr;
    struct tlb_entrie *head = *head_ptr;
    //start from the first entry
    struct tlb_entrie *current = head;
    while (current != NULL) {
        if (current->pg_num == page_num && current->ASID == pid) {
            current->valid = 0;
        }
        current = current->next;
    }
}

int get_N_offset(int pgsize) {
    int n = 0;
    while (pgsize != 1) {
        pgsize = pgsize / 2;
        n++;
    }
    return n;
}

/* Code Reference: https://www.tutorialspoint.com/data_structures_algorithms/linked_list_program_in_c.htm */
void initFreeFrameList(int physpages) {
    int i;
    for (i = physpages-1; i >= 0; i--) {
        //create a link
        struct free_fm *link = (struct free_fm*) malloc(sizeof(struct free_fm));
        link->fm_num = i;
        //point it to old first node
        link->next = free_fm_head;
        //point first to new first node
        free_fm_head = link;
        //printf("%s ", my_head->type);
        //return head;
    }
}

int useFreeFrame() {
    struct free_fm *new_head = free_fm_head->next;
    int rt_frame = free_fm_head->fm_num;
    free(free_fm_head);
    free_fm_head = new_head;
    return rt_frame;
}

void addFreeFrame(int frame) {
    //create a link
    struct free_fm *link = (struct free_fm*) malloc(sizeof(struct free_fm));
    link->fm_num = frame;
    //point it to old first node
    link->next = free_fm_head;
    //point first to new first node
    free_fm_head = link;
    //printf("%s ", my_head->type);
    //return head;
}



int main(int argc, char *argv[]){

    int i,ref;
    int pgsize, tlbentries, quantum, physpages, tlb_type, ev_policy;
    unsigned int pg_num;
    FILE **processes;

    // Process TLB
    struct tlb_entrie** TLB_heads;
    struct tlb_entrie** TLB_tails;

    // Global TLB
    // struct tlb_entrie* TLB_head;
    // struct tlb_entrie* TLB_tail;

    // physical memeory pointer
    struct phys_entry *phys_mem;

    // number of process
    int Nprocess = argc-7;
    // output statistic
    int t = 0;
    int tlbhits[Nprocess];
    int pf[Nprocess];
    int pageout[Nprocess];
    int current_resident_page[Nprocess];
    long total_resident_page[Nprocess];
    for (i = 0; i < Nprocess; i++) {
        tlbhits[i] = 0;
        pf[i] = 0;
        pageout[i] = 0;
        current_resident_page[i] = 0;
        total_resident_page[i] = 0;
    }

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
    if (strcmp(argv[3], "g") == 0) {
        tlb_type = GLOBAL;
    } else if (strcmp(argv[3], "p") == 0) {
        tlb_type = PROCESS;
    } else {
        fprintf(stderr, "ERROR: tlb_type\n");
        return(1);
    }

    // check page eviction policy (FIFO/LRU)
    if (strcmp(argv[6], "f")!=0 && strcmp(argv[6], "l")!=0) {
        fprintf(stderr, "The 7th argument should be { f | l }\n");
        return(1);
    }
    if (strcmp(argv[6], "f") == 0) {
        ev_policy = FIFO;
    } else if (strcmp(argv[3], "p") == 0) {
        ev_policy = LRU;
    } else {
        fprintf(stderr, "ERROR: Eviction policy\n");
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
    phys_mem = (struct phys_entry *) calloc(physpages, sizeof(struct phys_entry));
    // initalize a link list (list of free physical frame number)
    initFreeFrameList(physpages);

    // initalize TLB table
    if (tlb_type == PROCESS) {
        TLB_heads = (struct tlb_entrie**) malloc(sizeof(struct tlb_entrie*) * Nprocess);
        TLB_tails = (struct tlb_entrie**) malloc(sizeof(struct tlb_entrie*) * Nprocess);
        init_TLB_P(tlbentries, Nprocess, TLB_heads, TLB_tails);
    } else if (tlb_type == GLOBAL) {

        init_TLB_G(tlbentries);
    }

    // get number of offset/page bits
    int len_offset = get_N_offset(pgsize);
    int len_page = 32 - len_offset;

    // initalize Page table (one table for each process)
    struct page_entrie **all_pages = (struct page_entrie **) malloc(sizeof(struct page_entrie *) * Nprocess);
    for (i = 0; i < Nprocess; i++) {
        all_pages[i] = (struct page_entrie *) calloc((1 >> len_page), sizeof(struct page_entrie));
    }

    // reading memory references (address) from each file in cyclical order
    unsigned int address;
    int n, rt, fm_num, evicted_pg_num, evicted_pid;

    // all processes are not EOF at start
    int is_EOF[Nprocess];
    for (i = 0; i < Nprocess; i++) {
        is_EOF[i] = 0;
    }
    int running_p = Nprocess;
    int test = 0;
    // loop until all processes is EOF (running_p = 0)
    while (running_p > 0) {
        printf("%d\n", test);
        test++;
        for (i = 0; i < Nprocess; i++) {
            if (!is_EOF[i]) {
                for (ref = 0; ref < quantum; ref++) {
                    if (!fread(&address,4,1,processes[i])) {    // EOF
                        is_EOF[i] = 1;
                        running_p = running_p - 1;
                        // freeMemory() ................................................
                        break;
                    }
                    // increment counter of total number of reference ------------------
                    t = t + 1;
                    // -----------------------------------------------------------------

                    // right shift len_offset bit (get page number)
                    pg_num = address >> len_offset;

                    // lookup TLB table first
                    if (tlb_type == PROCESS) {
                        //printf("pos1\n");
                        rt = lookup_tlb(&TLB_heads[i], &TLB_tails[i], pg_num, 0);
                    } else if (tlb_type == GLOBAL) {
                        //printf("pos2\n");
                        rt = lookup_tlb(&TLB_head, &TLB_tail, pg_num, i);
                    }

                    // TLB miss
                    if (rt == 0 || rt == 1) {
                        // lookup page table
                        if (all_pages[i][pg_num].valid == 0) {      // page-fault
                            // increment page-fault (pf) counter
                            // ---------------------------------------------------------
                            pf[i] = pf[i] + 1;
                            // ---------------------------------------------------------

                            // if there is a free frame. use the free frame first
                            if (free_fm_head != NULL) {
                                fm_num = useFreeFrame();
                                addFrameReference(fm_num);
                            } else {
                                // else evict a frame
                                fm_num = evictFrameReference();
                            }

                            // evict memory
                            if (phys_mem[fm_num].valid == 0) {
                                // add occupied process-info to physical memory
                                phys_mem[fm_num].pg_num = pg_num;
                                phys_mem[fm_num].pid = i;
                                phys_mem[fm_num].valid = 1;

                            } else {
                                evicted_pg_num = phys_mem[fm_num].pg_num;
                                evicted_pid = phys_mem[fm_num].pid;

                                //increment pageout counter, decrease victim process's current_resident_page
                                // -------------------------------------------------------------------------
                                pageout[evicted_pid] = pageout[evicted_pid] + 1;
                                current_resident_page[evicted_pid] = current_resident_page[evicted_pid] - 1;
                                // -------------------------------------------------------------------------

                                // set evicted page table to invalid
                                all_pages[evicted_pid][evicted_pg_num].valid = 0;

                                // set evicted TLB table to invalid
                                if (tlb_type == PROCESS) {
                                    setEvictedEntryToInvalid_tlb(&TLB_heads[evicted_pid], &TLB_tails[evicted_pid], evicted_pg_num, 0);
                                } else if (tlb_type == GLOBAL) {
                                    setEvictedEntryToInvalid_tlb(&TLB_head, &TLB_tail, evicted_pg_num, evicted_pid);
                                }
                            }

                            // update page table (bring in new entry)
                            all_pages[i][pg_num].valid = 1;
                            all_pages[i][pg_num].fm_num = fm_num;

                            // increment current_resident_page of the swap-in process
                            // ---------------------------------------------------------
                            current_resident_page[i] = current_resident_page[i] + 1;
                            // ---------------------------------------------------------

                        }

                        // in physcial memory (page table is valid), it just not in TLB
                        else if (all_pages[i][pg_num].valid == 1) {
                            // if eviction policy is LRU -> "wipe the dust"
                            if (ev_policy == LRU) {
                                fm_num = all_pages[i][pg_num].fm_num;
                                updateFrameReference(fm_num);
                            }
                        }

                        // update TLB table (bring in new entry)
                        // if rt = 0, the entry does not in TLB table
                        if (tlb_type == PROCESS && rt == 0) {
                            update_tlb(&TLB_heads[i], &TLB_tails[i], pg_num, 0);
                        } else if (tlb_type == GLOBAL && rt == 0) {
                            update_tlb(&TLB_head, &TLB_tail, pg_num, i);
                        }
                    }

                    // TLB Hit
                    if (rt == 2) {
                        // increment tlbhits counter
                        tlbhits[i] = tlbhits[i] + 1;
                    }

                    // add up total_resident_page ------------------------------------------------
                    for (n = 0; n < Nprocess; n++) {
                        total_resident_page[n] = total_resident_page[n] + current_resident_page[n];
                    }
                    // ---------------------------------------------------------------------------


                }
            }
        }
    }


    for (i = 0; i < Nprocess; i++) {
        printf("Process%d: %d %d %d %ld\n", i+1, tlbhits[i], pf[i], pageout[i], total_resident_page[i] / t);
    }










}