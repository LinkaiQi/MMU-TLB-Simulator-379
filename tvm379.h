#define GLOBAL 0
#define PROCESS 1

#define FIFO 0
#define LRU 1

/*  ---------------------- Global Variables ---------------------- */
struct free_fm *free_fm_head = NULL;
struct reference *reference_head = NULL;
struct reference *reference_tail = NULL;
// (Global) TLB
struct tlb_entrie* TLB_head;
struct tlb_entrie* TLB_tail;
// (Process) TLB
struct tlb_entrie** TLB_heads;
struct tlb_entrie** TLB_tails;
// physical memeory pointer
struct phys_entry *phys_mem;
// list of page table pointers (each process has a page table)
struct page_entrie **all_pages;

/* ------------------- function declaration --------------------- */
// add frame reference to the head of the linked-list
void addFrameReference(int frame);
// LRU ONLY
void updateFrameReference(int frame);
// evict a frame reference
int evictFrameReference();
// initialize TLB (type = process)
void init_TLB_P(int tlbentries, int n_process);
// initialize TLB (type = Global)
void init_TLB_G(int tlbentries);
// if it is process TLB, pid pass as 0, else pid is the actual process id
int lookup_tlb_P(unsigned int page_num, int pid);
// Global lookup TLB
int lookup_tlb_G(unsigned int page_num, int pid);
// update TLB table (type = process)
void update_tlb_P(unsigned int page_num, int pid);
// update TLB table (type = Global)
void update_tlb_G(unsigned int page_num, int pid);
// evict entry from TLB table
void setEvictedEntryToInvalid_tlb(struct tlb_entrie **head_ptr, 
    struct tlb_entrie **tail_ptr, unsigned int page_num, int pid);
// calculate the number of offset bit
int get_N_offset(int pgsize);
// initialize free frame list
void initFreeFrameList(int physpages);
// return a free frame number from the freeFrameList
int useFreeFrame();
// add a free frame to the freeFrameList
void addFreeFrame(int frame);
// release memory after the process is terminated
void release_memory(int pid, int physpages, int tlb_type);


/* ------------------- structure defination --------------------- */
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
