/**
* @Author: Sanzida Hoque
* @Course: CMP_SCI 4760 Operating Systems 
* @Sources: https://www.geeksforgeeks.org/bakery-algorithm-in-process-synchronization/
*           https://www.tutorialspoint.com/inter_process_communication/inter_process_communication_shared_memory.htm
*/

#include "config.h"

int opt, nprocs = 20, terminate_time = 100, errno, pid = 0,  *parents = NULL, *children = NULL;
time_t startTime = 0;

int main(int argc, char *argv[])
{
    printf("master is processessing...\n");
    /* Interrupt signal handling */
    signal(SIGALRM, signal_timer);//Abort for end of termination time 
    signal(SIGINT, signal_abort);// Abort for Ctrl+C     
  
    /* Parsing Command Line argument */
    while ((opt = getopt(argc, argv, "t:n:h:")) != -1) {

        switch (opt) {
        case 'h':
            printf("Help:\n");
            printf("Run using master -t [ss] [n]\n");
            printf("number of process > 20 will be automatically set for 20.");
        case 't':
            terminate_time = atoi(optarg);
            if(terminate_time < 1){
 		perror("master: Termination time is not valid");
            }
            nprocs = atoi(argv[3]);
            if(nprocs > MAXPROC){
            	perror("master: Warning: Numbers of Processes can not exceed 20");
            	nprocs = 20;
            }
            break;
        case 'n':
            nprocs = atoi(optarg);
            if(nprocs > MAXPROC){
            	perror("master: Warning: Numbers of Processes can not exceed 20");
            	nprocs = 20;
            }
            break;
        case '?':
            if (opt == 'c')
                perror("master: Error: Option requires an argument");
            else
                perror("master: Error: Unknown option character");
            return 1;

        }

    }

    fprintf(stderr, "terminate_time: %d\n", terminate_time);
    fprintf(stderr, "nprocs: %d\n", nprocs); 
   
    parents = malloc(sizeof(int) * nprocs);
    children = malloc(sizeof(int) * nprocs);
    
    /* Creating a new shared memory segment */  
    // choose_id = shmget(ftok("Makefile", '1'), sizeof(struct shmseg), IPC_CREAT | S_IRUSR | S_IWUSR);
    choose_id = shmget(ftok("Makefile", '1'), sizeof(bool) * nprocs, IPC_CREAT | 0666);
    ticket_id = shmget(ftok("Makefile", '2'), sizeof(int) * nprocs, IPC_CREAT | 0666);
    cstest_id = shmget(ftok("Makefile", '3'), sizeof(FILE) * nprocs, IPC_CREAT | 0666);
    
    printf("ipcrm -m %d\n", choose_id);
    printf("ipcrm -m %d\n", ticket_id);
    printf("ipcrm -m %d\n", cstest_id);

    if (choose_id == -1 || ticket_id == -1 || cstest_id == -1){
        perror("master: Error: Shared memory allocation failed");
     // return 1;
    }

    choosing = (bool *)shmat(choose_id, NULL, 0);
    tickets = (int *)shmat(ticket_id, NULL, 0);
    cstest = (FILE *)shmat(ticket_id, NULL, 0);
    
   if (choosing == (void *) -1 || tickets == (void *) -1 || cstest == (void *) -1 ) {
       perror("master: Error: Shared memory attachment failed");
      //return 1;
   }

    for (i = 0; i < nprocs; i++){
        choosing[i] = false;
        tickets[i] = 0;
    }
    
    /* spawning child processes and waiting to finish */
    startTime = time(NULL);
    alarm(terminate_time);
    for (int i = 0; i < nprocs; i++){
        
        pid = fork();//spawn fan of child processes
        //printf("pid %d\n",pid);
	
	/* Checking fork */
	if (pid == -1){ // Something went wrong
	    perror("master: Error: Failed to create a child process");
	    exit(EXIT_FAILURE);
	}

        if (pid == 0){// Child processes
            char procid[256];
            sprintf(procid, "%d", i+1);
            char numprocs[256];
            sprintf(numprocs, "%d", nprocs);
            printf("child pid %d from the"
                   " parent pid %d, i = %d\n",
                getpid(), getppid(),i+1); 
            children[i] = pid; 
            errno = execl("./slave", "./slave" , procid, numprocs, NULL); // Processing slaves
            if(errno == -1){
                perror("master: Error: Can't process slave!");
            }
            exit(EXIT_SUCCESS);
        }
        else{ // Parent processes
            parents[i] = pid; 
        }

    }

    for (j = 0; j < nprocs; j++)
         wait(NULL);

    deallocate_shm(); // Deallocating Shared memory segment after completing waiting for child processes   
    exit(EXIT_SUCCESS);
}

/* Deallocate Shared Memory */
void deallocate_shm(){
  if (shmdt(choosing) == -1 || shmdt(tickets) == -1 || shmdt(cstest) == -1) {
      perror("master: Error: shmdt failed to detach memory");
  }
  if (shmctl(choose_id, IPC_RMID, 0) == -1 || shmctl(ticket_id, IPC_RMID, 0) == -1 || shmctl(cstest_id, IPC_RMID, 0) == -1 ) {
      perror("master: Error: shmctl failed to delete shared memory");
  }  
}

/* signal handle for time out */
void signal_timer(int signal){
	if(difftime(time(NULL),startTime) >= terminate_time){
	   perror("master: Warning: timer ends");
	   killpg((*parents), SIGTERM);
	   killpg((*children), SIGTERM);
	    for(int i = 0; i < nprocs; i++){
		wait(NULL);
	    }
	   deallocate_shm();
	   //abort();
	   exit(EXIT_FAILURE);
	}
}


/* signal handle for receiving CTRL + C */
void signal_abort(){ 
    perror("master: Warning: CTRL + C received, master is terminating");
    killpg((*parents), SIGTERM);
    killpg((*children), SIGTERM);
    for(int i = 0; i < nprocs; i++){
		wait(NULL);
	    }
    deallocate_shm();
    //abort();
    exit(EXIT_FAILURE);
}




