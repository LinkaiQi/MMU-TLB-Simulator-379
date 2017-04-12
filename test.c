#include <stdio.h>
#include <stdlib.h>

// the (page_entrie address - base address) / sizeof(page_entrie) is page number
struct page_entrie {
    // unsigned int pg_num;
    unsigned int fm_num;
    int valid;
};

int main(int argc, char *argv[]){
    int i;

    // initalize Page table (one table for each process)
    struct page_entrie **all_pages = (struct page_entrie **) malloc(sizeof(struct page_entrie *) * 10);
    for (i = 0; i < 10; i++) {
        all_pages[i] = (struct page_entrie *) calloc((4), sizeof(struct page_entrie));
    }

    all_pages[0][3].fm_num = 142;
    all_pages[8][1].fm_num = 666;


    printf("%d\n", all_pages[0][3].fm_num);
    printf("%d\n", all_pages[8][1].fm_num);

    printf("%lu\n", sizeof(unsigned int));


}
