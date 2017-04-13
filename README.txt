#---------------------------------------------------------------
# Assignment:           3
# Due Date:             April 12, 2017
# Name:                 Linkai Qi; Ting Wang
# CSID:                 linkai; ting8
# Lecture Section:      B1
# Instructor:           Ioanis Nikolaidis
# group number:         33
#----------------------------------------------------------------
# Brief description of this program:
This program is a simulator that simulates TLB behaviour and the virtual memory
page-in page-out eviction activity.

# How to use it:
1. compiling: $ make
2. run:
  ./tvm379 pgsize tlbentries { g | p } quantum physpages { f | l } trace1 trace2 ... tracen

  Note: "pgsize" is the page size (always a power of 2 from 16 to 65,536), "tlbentries"
        are the number of simulated TLB entries (always a power of 2 from 8 to 256),
        followed by a single letter indication whether the TLB is uniform across
        processes (g for “global”) or if they cannot distinguish across processes
        (p i.e., “process” specific). The "quantum" means the number of memory
        reference being read at each time from each file. Besides, "physpages" indicates
        the number of available physical memory page (up to 1,000,000), followed by a
        letter indication whether the page eviction policy is FIFO (f) of LRU (l).
        The last "trace1 trace2 ... tracen" is a variable number of trace files.
        Each of them contains a sequence of 32-bit memory reference.
3. delete and uninstall: $make clean

# About my simulator:

The simulator starts with initial three tables (TLB, Page Table, physical memory table)
 and a link to keep track of the least recently used frame number.

The TLB table has the data structure of Double-linked-list. At the beginning, it has "tlbentries" entries with page-number, frame-number, validation-number all equal to zero.
Since the TLB table is always LRU, when the process references an entry in the TLB, TLB will move the entry to the head of the table (which means "wipe the dust" analogy).
When we need to evict an entry, we discard the last entry (in the tail) of the TLB.

The page table is a sequential-list with increment page number. One page table for each process. The page table has columns of frame-number, and validation-number.
The (page-entry address - base page table address) / sizeof(entry) is page number. If validation-number equals to zero, then the page is no in the physical memory.

The physical memory table has the size of "physpages" frames. The physical table is empty at the start.
If a new page came in, the physical memory entry will record which process the new page belong to and what is the page number in the process's page table.
When the next time the entry become victim entry (need to be swapped out), the simulator can lookup the physical memory entry to know which entry in the page table and TLB need to update
There is also a linked-list associated with the physical memory table to keep track of eviction order (LRU / FIFO). If the eviction policy is LRU, the linked-list has structure same as the structure used in TLB table.
If the eviction policy is FIFO, the linked-list is a queue (first in first out).
