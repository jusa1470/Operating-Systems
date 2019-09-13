/*
 * File: pager-predict.c
 * Author:       Andy Sayler
 *               http://www.andysayler.com
 * Adopted From: Dr. Alva Couch
 *               http://www.cs.tufts.edu/~couch/
 *
 * Project: CSCI 3753 Programming Assignment 4
 * Create Date: Unknown
 * Modify Date: 2012/04/03
 * Description:
 * 	This file contains a predictive pageit
 *      implmentation.
 */

#include <stdio.h> 
#include <stdlib.h>

#include "simulator.h"

void LRU(Pentry q[MAXPROCESSES], int proc, int tick, int timestamps[MAXPROCESSES][MAXPROCPAGES]) {
    int i;
    int recent = -1;
    int recent_idx = -1;
    for (i = 0; i < q[proc].npages; i++) {
        /* Find biggest elapsed timestamp */
        int elapsed = tick - timestamps[proc][i];
        if (q[proc].pages[i] && (elapsed >= recent || recent == -1)) {
            recent = elapsed;
            recent_idx = i;
        }
    }
    pageout(proc, recent_idx);
}

void predict(int proc, int page, int prev[MAXPROCESSES][MAXPROCPAGES][MAXPROCPAGES]) {
    /* find most frequent next page */
    int i;
    for (i = 0; i < MAXPROCPAGES; i++) {
        if (prev[proc][page][i] > 0) {
            pagein(proc, i);
        }
    }
}

void pageit(Pentry q[MAXPROCESSES]) { 
    
    /* This file contains the stub for a predictive pager */
    /* You may need to add/remove/modify any part of this file */

    /* Static vars */
    static int initialized = 0;
    static int tick = 1; // artificial time
    static int timestamps[MAXPROCESSES][MAXPROCPAGES];
    static int prev[MAXPROCESSES][MAXPROCPAGES][MAXPROCPAGES];
    static int last_page[MAXPROCESSES];

    /* Local vars */
    int proctmp;
    int pagetmp;
    int pagetmp2;

    /* initialize static vars on first run */
    if(!initialized){
	/* Init complex static vars here */
        for(proctmp=0; proctmp < MAXPROCESSES; proctmp++){
            for(pagetmp=0; pagetmp < MAXPROCPAGES; pagetmp++){
                timestamps[proctmp][pagetmp] = 0;
                for (pagetmp2 = 0; pagetmp2 < MAXPROCPAGES; pagetmp2++) {
                    prev[proctmp][pagetmp][pagetmp2] = 0;
                }
            }
            last_page[proctmp] = 0;
        }
        initialized = 1;
    }

    
    /* TODO: Implement Predictive Paging */
    int pc;
    int page;
    int proc;
    
    for(proc = 0; proc < MAXPROCESSES; proc++) { 
        /* Is process active? */
        if(q[proc].active) {
            /* Dedicate all work to first active process*/ 
            pc = q[proc].pc;            // program counter for process
            page = pc/PAGESIZE;         // page the program counter needs
            timestamps[proc][page] = tick;

            /* If at end of page, pagein next page */
            if (pc % PAGESIZE == 0 || pc % PAGESIZE > PAGESIZE - 80) {
                pagein(proc, page + 1);
            }

            int prev_page = last_page[proc];
            last_page[proc] = page;

            if (prev_page != page) {
                prev[proc][prev_page][page]++;
                pageout(proc, prev_page);
            }
            int pred_page = (q[proc].pc + 85) / PAGESIZE;
            predict(proc, pred_page, prev);

            /* Is page swapped-out? */
            if(!q[proc].pages[page]) {
                /* Try to swap in */
                if(!pagein(proc,page)) {
                    /* If swapping fails, swap out another page */
                    LRU(q, proc, tick, timestamps);
                }
            }
        }
    } 

    /* advance time for next pageit iteration */
    tick++;
}