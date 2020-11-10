/* Define constants:
  - TRUE/FALSE: define true/false variables
  - MAXPROCPAGES: 20 virtual pages per process
  - MAXPROCESSES: 20 processes competing for pages
  - PAGESIZE: the size of each page is 128
  - PAGEWAIT: each page-in takes 100 ticks
  - PHYSICALPAGES: 100 physical pages
*/
#define TRUE 1
#define FALSE 0
#define MAXPROCPAGES 20
#define MAXPROCESSES 20
#define PAGESIZE 128
#define PAGEWAIT 100
#define PHYSICALPAGES 100
#define MAXPC (MAXPROCPAGES * PAGESIZE)


/* a structure for each program entry
  - active: 1 if process is running, 0 if process has exited
  - pc: ranges from 0-19; the current page is page = pc/PAGE_SIZE which also ranges from 0-19
  - npages: the number of pages in the processes memory space (MAX_PROC_PAGES if running, 0 if exited)
  - pages[MAXPROCPAGES]: 0 if page is swapped out/swapping out/swapping in, 1 if swapped in */
struct pentry{
  long active;
  long pc;
  long npages;
  long pages[MAXPROCPAGES];
};

typedef struct pentry Pentry;



/* pagein()
  - Arguments: int process = pc of the process
              int page = page to swap in
  - Returns: 1 = pagein() has started, pagein() is currently running, or pagein() has completed
            0 = swapping out */
extern int pagein(int process, int page);


/* pageout()
  - Arguments: same as above
  - Returns: 1 = pageout() has started, pageout() is currently running, or pageout() has completed
            0 = swapping in */
extern int pageout(int process, int page)


/* pageit()
  - Arguments: an array of pentry structures, containing information on each process */
extern void pageit(Pentry q[MAXPROCESSES]);










/* This library defines pageit(), pagein(), and pageout() */
#include "simulator.h"






void pageit(Pentry q[MAXPROCESSES]){

  /* Define variables */
  int pc;
  int page;

  /* Iterate through each process i */
  for(int i = 0; i < MAXPROCESSES; i++){

    /* Find the first active process */
    if(q[i].active){

      /* Determine current page */
      pc = q[i.pc];
      page = pc/PAGESIZE;

      /* See if virtual page is in physical memory or not
        - If virtual page is currently in physical memory, exit pageit()
        - If virtual page is NOT currently in physical memory, call pagein() */
      if(!q[i].pages[page]){

        /* Call pagein()
          - If pagein() returns success, exit pageit()
          - If pagein() returns failure, call pageout() */
        if(!pagein(i, page)){

          /* Call pageout() on every page until pageout() is success */
          for(int j = 0; j < q[i].npages; j++){
            if(j != page){
              if(pageout(i, j)){
                break;
              }
            }
          }
        }
      }

      /* Break for-loop after finding first active process */
      break;
    }
  }
}
