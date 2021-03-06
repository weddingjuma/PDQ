/*******************************************************************************/
/*  Copyright (c) 1994 - 2011, Performance Dynamics Company                    */
/*                                                                             */
/*  This software is licensed as described in the file COPYING, which          */
/*  you should have received as part of this distribution. The terms           */
/*  are also available at http://www.perfdynamics.com/Tools/copyright.html.    */
/*                                                                             */
/*  You may opt to use, copy, modify, merge, publish, distribute and/or sell   */
/*  copies of the Software, and permit persons to whom the Software is         */
/*  furnished to do so, under the terms of the COPYING file.                   */
/*                                                                             */
/*  This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY  */
/*  KIND, either express or implied.                                           */
/*******************************************************************************/

/*
 * MVA_Canon.c
 * 
 * Updated by NJG on Sat May 13 10:01:19 PDT 2006
 * Revised by NJG on Mon, Apr 2, 2007. MSQ erlang solver
 * Revised by NJG on Friday, June 26, 2009. See function: sumU(int k)
 * Revised by NJG on Tuesday, March 1, 2011. Set Dsat=0.0 in each c-loop iteration (Newsom)
 *
 *  $Id: MVA_Canon.c,v 4.10 2011/03/01 22:12:38 earl-lang Exp $
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "PDQ_Lib.h"

//-------------------------------------------------------------------------

void canonical(void)
{
    extern int        PDQ_DEBUG, streams, nodes;
    extern char       s1[], s2[], s3[], s4[];
    extern JOB_TYPE  *job;
    extern NODE_TYPE *node;
    extern double     ErlangR(double arrivrate, double servtime, int servers); // ANSI

    int               k;
    int               m; // servers in MSQ case
    int               c = 0;
    double            X;
    double            Xsat;
    double            Dsat = 0.0;
    double            Ddev;
    double            sumR[MAXSTREAMS];
    double            devU;
    double            sumU();
    char              jobname[MAXBUF];
    char              satname[MAXBUF];
    char             *p = "canonical()";

    if (PDQ_DEBUG)
        debug(p, "Entering");

    for (c = 0; c < streams; c++) {
        sumR[c] = 0.0;
        Dsat = 0.0; // Fix submitted by James Newsom, 23 Feb 2011
        //Otherwise, stream index can be compared to wrong (old) device index

        X = job[c].trans->arrival_rate;
               
        // find bottleneck node (== largest service demand)
        for (k = 0; k < nodes; k++) {
        	// Hope to remove single class restriction eventually
        	if (node[k].sched == MSQ && streams > 1) {
				sprintf(s1, "Only single PDQ stream allowed with MSQ nodes.");
				errmsg(p, s1);
            }
            Ddev = node[k].demand[c];
            if (node[k].sched == MSQ) {     // multiserver case
                m = node[k].devtype;        // contains number of servers > 0
                Ddev /= m;
            }
            if (Ddev > Dsat) {
                Dsat = Ddev;
                //Since we're about to fall out this k-loop
                //keep device name in case of error msg
                sprintf(satname, "%s", node[k].devname);
            }
        } // end of k-loop
        
        Xsat = 1.0 / Dsat;
        job[c].trans->saturation_rate = Xsat;

        if (Dsat == 0) {
            sprintf(s1, "Dsat = %3.3f", Dsat);
            errmsg(p, s1);
        }

        if (X > Xsat) {
        	getjob_name(jobname, c);
            sprintf(s1, 
            	"\nArrival rate %3.3f for stream \'%s\' exceeds saturation thruput %3.3f of node \'%s\' with demand %3.3f", 
            	X, jobname, Xsat, satname, Dsat
            );  
        	errmsg(p, s1);
        }
        
        for (k = 0; k < nodes; k++) {
            node[k].utiliz[c] = X * node[k].demand[c];
            if (node[k].sched == MSQ) {     // multiserver case
                m = node[k].devtype;            // recompute m in every k-loop
                node[k].utiliz[c] /= m;     // per server
            }

            devU = sumU(k); // sum all workload classes

            if (devU > 1.0) {
                sprintf(s1, "\nTotal utilization of node \"%s\" is %2.2f%% (> 100%%)",
                    node[k].devname,
                    devU * 100
                    );
                errmsg(p, s1);
            }

            if (PDQ_DEBUG)
                printf("Tot Util: %3.4f for %s\n", devU, node[k].devname);

            switch (node[k].sched) {
                case FCFS:
                case PSHR:
                case LCFS:
                    node[k].resit[c] = node[k].demand[c] / (1.0 - devU);
                    node[k].qsize[c] = X * node[k].resit[c];
                    break;
                case MSQ: // Added by NJG on Mon, Apr 2, 2007
                    node[k].resit[c] = ErlangR(X, node[k].demand[c], m);
                    node[k].qsize[c] = X * node[k].resit[c];
                    break;
                case ISRV:
                    node[k].resit[c] = node[k].demand[c];
                    node[k].qsize[c] = node[k].utiliz[c];
                    break;
                default:
                    typetostr(s1, node[k].sched);
                    sprintf(s2, "Unknown queue type: %s", s1);
                    errmsg(p, s2);
                    break;
            }
            
            sumR[c] += node[k].resit[c];
            
        }  // end of k-loop

        job[c].trans->sys->thruput = X;             // system throughput
        job[c].trans->sys->response = sumR[c];      // system response time 
        job[c].trans->sys->residency = X * sumR[c]; // total number in system

        if (PDQ_DEBUG) {
            getjob_name(jobname, c);
            printf("\tX[%s]: %3.4f\n", jobname, job[c].trans->sys->thruput);
            printf("\tR[%s]: %3.4f\n", jobname, job[c].trans->sys->response);
            printf("\tN[%s]: %3.4f\n", jobname, job[c].trans->sys->residency);
        }
        
    }  // end of c-loop

    if (PDQ_DEBUG)
        debug(p, "Exiting");

}  // canonical

//-------------------------------------------------------------------------

double sumU(int k)
{
	// Compute the total utilization for device k
	
    extern int        PDQ_DEBUG, streams, nodes;
    extern JOB_TYPE  *job;
    extern NODE_TYPE *node;


    int               c;
    double            sum = 0.0;
    char             *p = "sumU()";

    for (c = 0; c < streams; c++) {
    	// NJG on Sunday, June 28, 2009 7:29:45 PM
        // This branching is a hack. Why do I need it?
        // I think it's because multi-class workloads and multi-servers are incompatible.
         if (node[k].sched == MSQ) sum += node[k].utiliz[c];
         else sum += (job[c].trans->arrival_rate * node[k].demand[c]);
    }

    return (sum);
}   // sumU 

//-------------------------------------------------------------------------
