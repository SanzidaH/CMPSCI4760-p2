/**
* @Author: Sanzida Hoque
* @Course: CMP_SCI 4760 Operating Systems 
* @Sources: https://www.geeksforgeeks.org/bakery-algorithm-in-process-synchronization/
*           https://www.tutorialspoint.com/inter_process_communication/inter_process_communication_shared_memory.htm
*/

#include "config.h"

int main(int argc, char *argv[])
{
    printf("slave is processessing...\n");
 
    /* Parsing Command Line argument */  
    int procid = atoi(argv[1]); 
    int nprocs = atoi(argv[2]); 
    printf("%d %d\n", procid, nprocs);
   
    /* Ensuring that IPC_CREAT created the segment in master */
    choose_id = shmget(ftok("Makefile", '1'), sizeof(bool) * nprocs, IPC_EXCL);
    ticket_id = shmget(ftok("Makefile", '2'), sizeof(int) * nprocs, IPC_EXCL);    
    cstest_id = shmget(ftok("Makefile", '3'), sizeof(FILE) * nprocs, IPC_EXCL);

    if (choose_id == -1 || ticket_id == -1 || cstest_id == -1){
        perror("slave: Error: Shared memory allocation failed");
     // return 1;
    }
    
    choosing = (bool *)shmat(choose_id, NULL, 0);
    tickets = (int *)shmat(ticket_id, NULL, 0);
    cstest = (FILE *)shmat(cstest_id, NULL, 0);
    
   if (choosing == (void *) -1 || tickets == (void *) -1 || cstest == (void *) -1 ) {
      perror("slave: Error: Shared memory attachment failed");
      //return 1;
   }

    char logfname[256];
    sprintf(logfname, "logfile.%d", procid);

    time_t curtime;
    struct tm *loc_time;
    char buf[LEN];
    int random_t = 0;


    for ( i = 0; i < 5; i++){
        srand(time(NULL) + procid + i); // seed for random sleep time
        int thread = procid - 1;
        int max_ticket = find_maximum(tickets, nprocs);
        choosing[thread] = 1;// Before getting the ticket number, choosing variable is set to be true
        tickets[thread] = max_ticket + 1; // Allotting a new ticket value as MAXIMUM + 1
        choosing[thread] = 0;// After getting the ticket number, choosing variable is set to be false

        /* ENTRY Section*/
        for (int other = 0; other < nprocs; other++){  
           /* bakery algorithm conditions */
            while (choosing[other]){}
            while ((tickets[other] != 0) && (tickets[other] < tickets[thread] || (tickets[other] == tickets[thread] && other < thread))){}
        }
      
        logfile = fopen(logfname, "a");
	 /* Critical Section */
        cstest = fopen("cstest", "a");
        if (cstest == NULL || logfile == NULL){
            perror("slave: Error: file open failed");
        }
        
        /* Logging */       
        curtime = time (NULL); //Getting current time of system  	
   	loc_time = localtime (&curtime);// Converting current time to local time
        fprintf(logfile, "Process id %d enters critical section %s\n", procid, asctime (loc_time));
        
        /* wait for a random number of seconds before writing */
        random_t = (rand() % 5) + 1; // random numbers in range [1, 5]
        sleep(random_t);
 	curtime = time (NULL); //Getting current time of system  	
   	loc_time = localtime (&curtime);// Converting current time to local time
        fprintf(logfile, "Process id %d writes on shared resource, cstest %s\n", procid, asctime (loc_time));
        strftime (buf, LEN, "%H:%M:%S", loc_time);// formatting time
        fprintf(cstest, "%s Queue %d File modified by process number %d\n", buf, tickets[procid - 1], procid);
       
	/* wait for a random number of seconds after writing */
        random_t = (rand() % 5) + 1; // random numbers in range [1, 5]
        sleep(random_t);
	curtime = time (NULL); //Getting current time of system  	
   	loc_time = localtime (&curtime);// Converting current time to local time
        fprintf(logfile, "Process id: %d exits critical section %s\n", procid, asctime (loc_time));
       
       /* closing files */
        fclose(logfile);
        fclose(cstest);
       
        /* EXIT Section */
        tickets[procid - 1] = 0;//Setting ticket value as lowest value after completion of process
    }

    deallocate_shm();
    exit(EXIT_SUCCESS);
}

/* Deallocate Shared Memory */
void deallocate_shm(){

    if (shmdt(choosing) == -1 || shmdt(tickets) == -1 || shmdt(cstest) == -1) {
        perror("slave: Error: shmdt failed to detach memory");
     }
    if (shmctl(choose_id, IPC_RMID, 0) == -1 || shmctl(ticket_id, IPC_RMID, 0) == -1 || shmctl(cstest_id, IPC_RMID, 0) == -1 ) {
        perror("slave: Error: shmctl failed to delete shared memory");
     }  
}

/* find maximum ticket value */ //https://www.geeksforgeeks.org/c-program-find-largest-element-array/
int find_maximum(int *arr, int n){ 
    int max = 0;
    for (c = 0; c < n; c++){
        if (max < arr[c]){
            max = arr[c];
        }
    }
    return max;
}

