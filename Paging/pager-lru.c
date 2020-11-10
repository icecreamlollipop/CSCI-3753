/*
 * File: pager-lru.c
 * Author:       Andy Sayler
 *               http://www.andysayler.com
 * Adopted From: Dr. Alva Couch
 *               http://www.cs.tufts.edu/~couch/
 *
 * Project: CSCI 3753 Programming Assignment 4
 * Create Date: Unknown
 * Modify Date: 2012/04/03
 * Description:
 * 	This file contains an lru pageit
 *      implmentation.
 */

#include <stdio.h>
#include <stdlib.h>

#include "simulator.h"

void pageit(Pentry q[MAXPROCESSES]) {

    /* This file contains the stub for an LRU pager */
    /* You may need to add/remove/modify any part of this file */

    /* Static vars */
    static int initialized = 0;
    static int tick = 1; // artificial time
    static int timestamps[MAXPROCESSES][MAXPROCPAGES];

    /* Local vars */
    int proctmp;
    int pagetmp;
    int i, mypg, j, min_time, old;

    /* initialize static vars on first run */
    if(!initialized){
	for(proctmp=0; proctmp < MAXPROCESSES; proctmp++){
	    for(pagetmp=0; pagetmp < MAXPROCPAGES; pagetmp++){
		timestamps[proctmp][pagetmp] = 0;
	    }
	}
	initialized = 1;
    }



    for(i = 0; i < MAXPROCESSES; i++) //traverse processes
    {
    	if(q[i].active){
    		mypg = q[i].pc/PAGESIZE;  // given that this is how to calculate the current page
    		// if page isn't already in, swap it in and prepare another to be kicked
    		if(!q[i].pages[mypg] && !pagein(i, mypg)){
				min_time = tick;
				// looping through pages within the process
				for(j = 0; j < MAXPROCPAGES; j++)
				{
					// if page at j is older than our original min_time, reassign
					if(timestamps[i][j] < min_time && q[i].pages[j])
					{
						// new oldest page time
						min_time = timestamps[i][j];
						// oldest page
						old = j;
					}
				}

				pageout(i, old);
				break;
    		}
    		timestamps[i][pagetmp] = tick;
    	}
    }
    /* TODO: Implement LRU Paging */
    // fprintf(stderr, "pager-lru not yet implemented. Exiting...\n");
    // exit(EXIT_FAILURE);

    /* advance time for next pageit iteration */
    tick++;
}
