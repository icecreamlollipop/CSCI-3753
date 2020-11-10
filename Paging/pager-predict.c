#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#include "simulator.h"

/*
we form predictions of behavior of processes with our matrix by tracking
movements from one page to another by certain processes. each process has an array
of size MAXPROCPAGES and for each of those pages we store any page that the process moved to from
that page
*/
void predict(int pc, int proc, int pc_prev, int pred_matrix[MAXPROCESSES][MAXPROCPAGES][MAXPROCPAGES]){
    int i;
    int *moves;

    //new array to track movement of current process to pages
    moves = pred_matrix[proc][pc_prev];

    for(i = 0; i < MAXPROCPAGES; i++){

        if((moves[i] == -1)|(moves[i] == pc)){
            // add the page transition into array
            moves[i] = pc;
            break;
        }
    }
}






void pageit(Pentry q[MAXPROCESSES]) {

    /* Static vars */
    static int initialized = 0;
    static int tick = 1; // artificial time
    static int timestamps[MAXPROCESSES][MAXPROCPAGES];
    static int pgs_prev[MAXPROCESSES]; //store the previous page counter so we can track movements

    //matrix to store the movement of processes from one page to another
    static int pred_matrix[MAXPROCESSES][MAXPROCPAGES][MAXPROCPAGES];

    /* Local vars */
    int proc, page, old, pg_prev, i, mypg, j, min_time;

    /* initialize static vars on first run */
    if(!initialized){
        int k;
        for(proc = 0; proc < MAXPROCESSES; proc++){
            for(page = 0; page < MAXPROCPAGES; page++){
                timestamps[proc][page] = 0;
                for(k = 0; k < MAXPROCPAGES; k++){
                    pred_matrix[proc][page][k] = -1;
                }
            }
        }
        initialized = 1;
    }

    // use current and previous page to modify the markov matrix
    for(proc = 0; proc < MAXPROCESSES; proc++){ //go through all processes
        if(q[proc].active){
            if(pg_prev != -1){
                pg_prev = pgs_prev[proc];
                //save previous process
                pgs_prev[proc] = q[proc].pc/PAGESIZE;
                // save current process
                page = q[proc].pc/PAGESIZE;
                if(pg_prev != page){
                    pageout(proc, pg_prev);
                    // this is how we'll get a page prediction
                    predict(page, proc, pg_prev, pred_matrix);
                }
            }
        }
    }

    // LRU in case prediction fails
    for(i = 0; i < MAXPROCESSES; i++){
        if(q[i].active){
            mypg = q[i].pc/PAGESIZE;  // given that this is how to calculate the current page
            // if page isn't already in, swap it in and prepare another to be kicked
            if(!q[i].pages[mypg] && !pagein(i, mypg)){
                min_time = tick;
                // looping through pages within the process
                for(j = 0; j < MAXPROCPAGES; j++){
                    // if page at j is older than our original min_time, reassign
                    if(timestamps[i][j] < min_time && q[i].pages[j]){
                        // new oldest page time
                        min_time = timestamps[i][j];
                        // oldest page
                        old = j;
                    }
                }
                pageout(i, old);
                break;
            }
            timestamps[i][page] = tick;
        }
    }

    // predictively paging in pages based on the matrix built (if it has values for us to use to make predictions)
    for(proc = 0; proc < MAXPROCESSES; proc++){
        int *pages;
        int future_page; //possible future page
        int i;
        if(q[proc].active){

            // try to predict the future page
            future_page = (q[proc].pc + 101)/PAGESIZE;

            pages = pred_matrix[proc][future_page];

            // page in the future pages
            for(i = 0; i < MAXPROCPAGES; i++){
                if(pages[i] == -1){
                    break;
                    // we have no data to predict
                }
                else{
                  pagein(proc, pages[i]);
                  // predictively adding pages into table
                }
            }
        }
    }

    tick++;
}
