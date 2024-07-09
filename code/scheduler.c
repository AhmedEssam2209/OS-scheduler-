#include "headers.h"
#include "process.h"
#include "PriorityQueue.h"
#include "Queue.h"
#include <math.h> 
#define MAX_BLOCKS 1000 // Define a constant for the maximum number of blocks
int map_mem[1000];
typedef struct Pair { // Define a structure for a pair of integers
    int lb, ub; // Lower bound and upper bound of a memory block
} Pair;

typedef struct Buddy { // Define a structure for the Buddy system
    int size; // Total size of the memory
    Pair* arr[MAX_BLOCKS]; // Array of free blocks for each size
    int arr_count[MAX_BLOCKS]; // Count of free blocks for each size
    int hm[MAX_BLOCKS]; // Hashmap to keep track of allocated blocks
} Buddy;

void printBuddySystem(Buddy* buddy) {
    printf("Buddy System:\n");
    for (int i = 0; i <= ceil(log(buddy->size) / log(2)); i++) {
        printf("Size %d:\n", (int) pow(2, i));
        for (int j = 0; j < buddy->arr_count[i]; j++) {
            Pair block = buddy->arr[i][j];
            printf("Block %d: Start = %d, End = %d\n", j+1, block.lb, block.ub);
        }
    }
}
Buddy* initialize(int s) { // Function to initialize the Buddy system
    Buddy* buddy = malloc(sizeof(Buddy)); // Allocate memory for the Buddy system
    buddy->size = s; // Set the total size of the memory
    int x = ceil(log(s) / log(2)); // Calculate the maximum size of a block
    for (int i = 0; i <= x; i++) 
    { // For each size up to the maximum size
        buddy->arr[i] = malloc(MAX_BLOCKS * sizeof(Pair)); // Allocate memory for the array of free blocks
        buddy->arr_count[i] = 0; // Initialize the count of free blocks to 0
    }
    buddy->arr[x][0].lb = 0; // Set the lower bound of the first block
    buddy->arr[x][0].ub = s - 1; // Set the upper bound of the first block
    buddy->arr_count[x] = 1; // Set the count of free blocks to 1
    for (int i = 0; i < MAX_BLOCKS; i++) { // For each possible block
        buddy->hm[i] = -1; // Initialize the hashmap to indicate that the block is not allocated
    }
    return buddy; // Return the initialized Buddy system
}
int allocate(FILE *M_log , Process P,Buddy * buddy, int s) // Function to allocate a block of memory
{
    int x = ceil(log(s) / log(2)); // Calculate the size of the block to be allocated
    int i; // Declare a variable for the index of the block
    Pair temp; // Declare a variable for the block to be allocated
    if (buddy->arr_count[x] > 0) { // If there is a free block of the required size
        temp = buddy->arr[x][0]; // Assign the first block of the current size to temp
        buddy->arr[x][0] = buddy->arr[x][--buddy->arr_count[x]]; // Move the last block of the current size to the first position
        printf("Memory from %d to %d allocated\n", temp.lb, temp.ub); // Print the range of the allocated memory block
        fprintf(M_log, "#At time %d allocated %d bytes for process %d from %d to %d \n",getClk(),s,P.id,temp.lb,temp.ub);
        buddy->hm[temp.lb] = temp.ub - temp.lb + 1; // Update the hashmap with the size of the allocated block
        return temp.lb; // Exit the function
    }
    for (i = x + 1; i < MAX_BLOCKS && buddy->arr_count[i] == 0; i++); // Find the next available block of a larger size
    if (i == MAX_BLOCKS) { // If no free block was found
        //printf("Sorry, failed to allocate memory\n"); // Print an error message
        return -1; // Exit the function
    }
    temp = buddy->arr[i][0]; // Assign the first block of the current size to temp
    buddy->arr[i][0] = buddy->arr[i][--buddy->arr_count[i]]; // Move the last block of the current size to the first position
    for (i--; i >= x; i--) { // For each size smaller than the current size
        Pair newPair = {temp.lb, temp.lb + (temp.ub - temp.lb) / 2}; // Split the block in half and create a new pair for the left half
        Pair newPair2 = {temp.lb + (temp.ub - temp.lb + 1) / 2, temp.ub}; // Create a new pair for the right half of the block
        buddy->arr[i][buddy->arr_count[i]++] = newPair; // Split the block in half and add the left half to the array of free blocks
        buddy->arr[i][buddy->arr_count[i]++] = newPair2; // Add the right half to the array of free blocks
        temp = buddy->arr[i][0]; // Update temp to the first block of the current size
        buddy->arr[i][0] = buddy->arr[i][--buddy->arr_count[i]]; // Move the last block of the current size to the first position
    }
    printf("Memory from %d to %d allocated\n", temp.lb, temp.ub); // Print the range of the allocated memory block
    fprintf(M_log, "#At time %d allocated %d bytes for process %d from %d to %d \n",getClk(),s,P.id,temp.lb,temp.ub);
    buddy->hm[temp.lb] = temp.ub - temp.lb + 1; // Update the hashmap with the size of the allocated block
    return temp.lb; // Return the address of the allocated block
}

void deallocate(FILE *M_log,Process P,Buddy * buddy, int s) 
{ 
    if (buddy->hm[s] == -1) {
        printf("Sorry, invalid free request\n");
        return;
    }
    int x = ceil(log(buddy->hm[s]) / log(2));
    int buddyNumber = s / buddy->hm[s];
    int buddyAddress = (buddyNumber % 2 != 0) ? s - pow(2, x) : s + pow(2, x);

    buddy->arr[x][buddy->arr_count[x]++] = (Pair){s, s + pow(2, x) - 1};
    fprintf(M_log, "#At time %d Freed %d bytes from process %d from %d to %d \n",getClk(),map_mem[P.id],P.id,s,s+buddy->hm[s]-1);   
    int coalescingDone = 1;
    while (coalescingDone) {
        coalescingDone = 0;
        for (int i = 0; i < buddy->arr_count[x]; i++) { 
            if (buddy->arr[x][i].lb == buddyAddress) { 
                int mergedAddress = (buddyNumber % 2 == 0) ? s : buddyAddress;
                buddy->arr[x + 1][buddy->arr_count[x + 1]++] = (Pair){mergedAddress, mergedAddress + 2 * pow(2, x) - 1};
                printf("Coalescing of blocks starting at %d and %d was done\n", s, buddyAddress);

                buddy->arr[x][i] = buddy->arr[x][--buddy->arr_count[x]]; 

                for (int j = 0; j < buddy->arr_count[x]; j++) {
                    if (buddy->arr[x][j].lb == s) {
                        buddy->arr[x][j] = buddy->arr[x][--buddy->arr_count[x]];
                        break;
                    }
                }

                buddy->hm[buddyAddress] = -1;
                coalescingDone = 1;

                // Update s, x, buddyNumber, and buddyAddress
                s = mergedAddress;
                x++;
                buddyNumber = s / pow(2, x);
                buddyAddress = (buddyNumber % 2 != 0) ? s - pow(2, x) : s + pow(2, x);
                break;
            }
        }
        
    }
    ;
    buddy->hm[s] = -1;
}
int currentlyrunning;
int detailsId;
int finishedId;
int qId;
int runId;
int map[1000];

PriorityQueue *PrQueueP;
Queue *QRR;

PriorityQueue *PrQueueS;

Process *PCBlock;


// Perf File

int totalRunningTime = 0;
double TotalWTA = 0;
int totalWaitingTime = 0;
int finalclk = 0;
int noProcesses;
double *WTA_Arr;
int WTA_Arr_idx = 0;

void Writeperf() // running time = burst time
{
    FILE *perfFile;
    //printf("final clk ::::: %d \n", finalclk);

    //printf("Totalwta ::::: %f \n", TotalWTA);
    perfFile = fopen("scheduler.perf", "w");
    fprintf(perfFile, "CPU utilization = %0.2f%c \n", (totalRunningTime / (float)finalclk) * 100, '%');
    fprintf(perfFile, "Avg WTA = %0.2f \n", TotalWTA / (float)noProcesses);
    double avg = TotalWTA / noProcesses;
    fprintf(perfFile, "Avg Waiting = %0.2f \n", totalWaitingTime / (float)noProcesses);
    double sd = 0;
    for (int i = 0; i < noProcesses; i++)
    {
        sd += pow((WTA_Arr[i] - avg), 2);
    }
    fprintf(perfFile, "Std WTA = %0.2f \n", sd / noProcesses);

    fclose(perfFile);
}

///



// Function to remove a process from an array of processes
void removeProcess(Process processes[], int *size, int indexToRemove)
{
    // Shift elements to the left to fill the gap left by the removed process
    for (int i = indexToRemove; i < *size - 1; i++)
    {
        processes[i] = processes[i + 1];
    }

    // Decrement the size of the array
    (*size)--;
}

void createprocess(Process received, Process PCBlock[], int processIndex)
{

    PCBlock[processIndex] = received;
    int processPID = fork();
    if (processPID == 0)
    {
        char ids[10], arrivals[10], finishTimes[10], isRunnings[10], prioritys[10], remainingTimes[10], runTimes[10], startTimes[10], waitingTime[10];
        snprintf(ids, sizeof(ids), "%d", received.id);
        snprintf(arrivals, sizeof(arrivals), "%d", received.arrival);
        snprintf(finishTimes, sizeof(finishTimes), "%d", received.finishTime);
        snprintf(isRunnings, sizeof(isRunnings), "%d", received.isRunning);
        snprintf(prioritys, sizeof(prioritys), "%d", received.priority);
        snprintf(remainingTimes, sizeof(remainingTimes), "%d", received.remainingTime);
        snprintf(runTimes, sizeof(runTimes), "%d", received.runTime);
        snprintf(startTimes, sizeof(startTimes), "%d", received.startTime);
        snprintf(waitingTime, sizeof(waitingTime), "%d", received.waitingTime);

        execl("./process.out", "./process.out", ids, remainingTimes, NULL);
    }
}

// send id, remaining time when forking

void HPF(PriorityQueue *pq)
{
    struct msgbuff CurrRun;
    // IF NOT EMPTY
    Process Running = pop(pq);
    Running.isRunning = 1;
    CurrRun.mprocess = Running;
    CurrRun.mtype = CurrRun.mprocess.id;
    msgsnd(runId, &CurrRun, sizeof(CurrRun.mprocess), !IPC_NOWAIT);
}

Process SRTN(PriorityQueue *pq)
{

    struct msgbuff CurrRun;
    // IF NOT EMPTY
    Process Running = pop(pq);
    Running.isRunning = 1;
    CurrRun.mprocess = Running;
    CurrRun.mtype = CurrRun.mprocess.id;
    msgsnd(runId, &CurrRun, sizeof(CurrRun.mprocess), !IPC_NOWAIT);
    return Running;
}

void SRTNback(PriorityQueue *pq, Process running)
{

    struct msgbuff CurrRun;
    // IF NOT EMPTY
    
    running.isRunning = 0;
    CurrRun.mprocess = running;
    CurrRun.mtype = CurrRun.mprocess.id;
    msgsnd(runId, &CurrRun, sizeof(CurrRun.mprocess), !IPC_NOWAIT);
}

void RR(Queue *q, int ind)
{
    struct msgbuff CurrRun;
    // IF NOT EMPTY
    Process Running = getElementAtIndex(q, ind);
    Running.isRunning = 1;
    CurrRun.mprocess = Running;
    CurrRun.mtype = CurrRun.mprocess.id;
    msgsnd(runId, &CurrRun, sizeof(CurrRun.mprocess), !IPC_NOWAIT);
}

void RRback(Queue *q, int ind)
{
    struct msgbuff CurrRun;
    // IF NOT EMPTY
    Process Running = getElementAtIndex(q, ind);
    Running.isRunning = 0;
    CurrRun.mprocess = Running;
    CurrRun.mtype = CurrRun.mprocess.id;
    msgsnd(runId, &CurrRun, sizeof(CurrRun.mprocess), !IPC_NOWAIT);
}

void receivePaused()
{
    // Receive processes coming back from SRTN OR RR TO ADD BACK TO THEIR QUEUES
    // Printouts here
    key_t key = ftok("keyfile", 0);
    int Stopped = msgget(key, 0666 | IPC_CREAT);
    struct msgbuff Stpmessagerec;
    while (true)
    {
        int receiveValue = msgrcv(Stopped, &Stpmessagerec, sizeof(Stpmessagerec.mprocess), 1, IPC_NOWAIT);
        if (receiveValue != -1)
        {
            printf("stopped/paused process with id= %d at time=%d", Stpmessagerec.mprocess.id, getClk());
            // if(Stpmessagerec.algo==2){

            // push(PrQueueS,Stpmessagerec.mprocess,Stpmessagerec.mprocess.runTime);//if algo was srtn then push stopped process into ssrtn queue
            //}else{
            //    enqueue(QueueRR,Stpmessagerec.mprocess); //if algo was rr then enqeue in rr queue
            //}
        }
    }
}

int receiveFinished(FILE *log,FILE *M_log ,Buddy* buddy,int map[],PriorityQueue *Not_alloc,int algo)
{
    struct msgbuff Fmessagerec;
    // Receive Finished processes
    // Printouts here
    // delete data of finished process

    int receiveValue = msgrcv(finishedId, &Fmessagerec, sizeof(Fmessagerec.mprocess), 0, !IPC_NOWAIT);
    
    if (receiveValue != -1 && Fmessagerec.mprocess.id != -1)
    {
        //
        int ta = getClk() - PCBlock[Fmessagerec.mprocess.id - 1].arrival;
        double wta = ta * 1.0 / (Fmessagerec.mprocess.runTime);
        totalRunningTime += Fmessagerec.mprocess.runTime;
        totalWaitingTime += Fmessagerec.mprocess.waitingTime;
        TotalWTA += wta;
        WTA_Arr[WTA_Arr_idx] = ((int)(wta * 100)) / 100.0;
        WTA_Arr_idx++;
        finalclk = getClk() -1;

        //

        printf("deallocating at position %d\n",map[Fmessagerec.mprocess.id]);
        fflush(stdout);
        deallocate(M_log,Fmessagerec.mprocess,buddy, map[Fmessagerec.mprocess.id]);
        //fprintf(M_log, "#At time %d Freed %d bytes for process %d from %d to %d \n",getClk(),Fmessagerec.mprocess.memsize,Fmessagerec.mprocess.id,map[Fmessagerec.mprocess.id],map[Fmessagerec.mprocess.id]+Fmessagerec.mprocess.memsize-1);
        fflush(M_log);
        while (true)
        {   
            int x;
            Process temp;
            
            if (Not_alloc->size > 0)
            {
                temp = pop(Not_alloc);
                printf("trying to allocate Process : %d with size :%d\n",temp.id,temp.memsize);
                fflush(stdout);
                x = allocate(M_log,temp,buddy, temp.memsize);
                if (x != -1)
                {
                    map[temp.id] = x;
                    //fprintf(M_log, "#At time %d allocated %d bytes for process %d from %d to %d \n",getClk(),temp.memsize,temp.id,x,x+temp.memsize-1);
                    switch (algo)
                    {
                    case 1:
                        push(PrQueueP, temp, temp.priority);
                        break;
                    case 2:
                        //push(PrQueueS, temp, temp.runTime);
                        break;
                    case 3:
                        pushq(QRR, temp);
                        break;
                    default:
                        break;
                    }
                    break;
                }
                else
                {
                    push(Not_alloc, temp, temp.memsize);
                    break;
                }
            }
            else
            {
                break;
            }

        }
        printBuddySystem(buddy);
        fprintf(log, "At time %d process %d finished arr %d total %d remain %d wait %d ta %d wta %.2f\n", getClk(), Fmessagerec.mprocess.id, PCBlock[Fmessagerec.mprocess.id - 1].arrival, Fmessagerec.mprocess.runTime, Fmessagerec.mprocess.remainingTime, Fmessagerec.mprocess.waitingTime, ta, wta);
        fflush(log);
        fflush(M_log);

        return 0;
    }
    return 1;
}

void receiveDetails(int stop, FILE *log)
{
    struct msgbuff dmessagerec;
    // Receive Finished processes
    // Printouts here
    // delete data of finished process

    int receiveValue = msgrcv(detailsId, &dmessagerec, sizeof(dmessagerec.mprocess), 0, !IPC_NOWAIT);
    if (receiveValue != -1)
    {

        // delete from pcb
        if (stop == 1)
            fprintf(log, "at time %d process %d stops arr %d total %d remain %d wait %d\n", getClk(), dmessagerec.mprocess.id, PCBlock[dmessagerec.mprocess.id - 1].arrival, dmessagerec.mprocess.runTime, dmessagerec.mprocess.remainingTime, dmessagerec.mprocess.waitingTime);
        else
            fprintf(log, "at time %d process %d starts arr %d total %d remain %d wait %d\n", getClk(), dmessagerec.mprocess.id, PCBlock[dmessagerec.mprocess.id - 1].arrival, dmessagerec.mprocess.runTime, dmessagerec.mprocess.remainingTime, dmessagerec.mprocess.waitingTime);
        fflush(log);
    }
}

int main(int argc, char *argv[])
{
    initClk();
   
    Buddy *buddy = initialize(1024); // Initialize the Buddy system with a total size of 1024
    FILE *log = fopen("scheduler.log", "w");
    FILE *M_log = fopen("memory.log", "w");
    printf("Entered scheduler!!\n");
    fflush(stdout);
    // TODO implement the scheduler :)

    if (argc != 4)
    {
        printf("Error in receiving!\n");
        fflush(stdout);
        return 1;
    }

    fprintf(log, "#At time x process y state arr w total z remain y wait k\n");
    fprintf(M_log, "#At time x allocated y bytes for process z from i to j \n");
    fflush(M_log);
    char *schedAlgo = argv[1];
    char *quantum = argv[2];
    char *countString = argv[3];
    PriorityQueue *Not_Alloc = (PriorityQueue *)malloc(sizeof(PriorityQueue));
    Not_Alloc->size = 0;

    int schedulerAlgo = atoi(schedAlgo);
    int q = atoi(quantum);
    noProcesses = atoi(countString);
    WTA_Arr=malloc(noProcesses*sizeof(double));
    int processIndex = 0;
    currentlyrunning = 0;
    // Process list to hold incoming processes
    PCBlock = (Process *)malloc(noProcesses * sizeof(Process));
    // initializeQueue(QueueRR);

    // Create message queue
    key_t key = ftok("keyfile", 0);
    qId = msgget(key, 0666 | IPC_CREAT);
    struct msgbuff qmessage;

    key_t keyy = ftok("keyfile", 'd');
    detailsId = msgget(keyy, 0666 | IPC_CREAT);
    struct msgbuff detailsmessage;

    key_t keyyy = ftok("keyfile", 'f');
    finishedId = msgget(keyyy, 0666 | IPC_CREAT);
    struct msgbuff finishedmessage;

    key_t keyyyy = ftok("keyfile", 'r');
    runId = msgget(keyyyy, 0666 | IPC_CREAT);
    struct msgbuff runmessage;

    int cc = 0;

    switch (schedulerAlgo)
    {
    case 1:

        PrQueueP = (PriorityQueue *)malloc(sizeof(PriorityQueue));
        PrQueueP->size = 0;
        
        // QUEUE FROM PCBLOCK TO PRIORITY QUEUE struct msgbuff message;
        while (true)
        {
            if (currentlyrunning == 1)
            {
                if (receiveFinished(log,M_log,buddy,map,Not_Alloc,schedulerAlgo) == 0)
                {
                   
                    currentlyrunning = 0;
                    cc++;
                if (cc==noProcesses)
                {
                  
                   printBuddySystem(buddy);
                    Writeperf();
                    msgctl(qId, IPC_RMID, NULL);
                    msgctl(detailsId, IPC_RMID, NULL);
                    msgctl(finishedId, IPC_RMID, NULL);
                    msgctl(runId, IPC_RMID, NULL);
                    printf("Scheduler is terminating\n");
                    fflush(stdout);
                    execl("killall process.out","killall process.out", NULL);
                    destroyClk(true);
                    
                    kill(0, SIGTERM);
                    exit(EXIT_SUCCESS);
                }
                    if (currentlyrunning == 0 && PrQueueP->size > 0)
                    {
                        HPF(PrQueueP);
                        currentlyrunning = 1;
                        receiveDetails(0, log);
                    }
                }
            }
            while (msgrcv(qId, &qmessage, sizeof(qmessage.mprocess), 0, !IPC_NOWAIT) != -1 && qmessage.mprocess.id != -1)
            {
                printf("Receiving %d\n", qmessage.mprocess.id);
                fflush(stdout);
                createprocess(qmessage.mprocess, PCBlock, processIndex);
                processIndex++;
                // send to hpf queue
                printf("HPF  -  trying to allocate Process : %d with size :%d\n",qmessage.mprocess.id,qmessage.mprocess.memsize);
                fflush(stdout);
                int x = allocate(M_log,qmessage.mprocess,buddy, qmessage.mprocess.memsize);
                if (x != -1)
                {
                  printBuddySystem(buddy);
                  map[qmessage.mprocess.id] = x;
                  map_mem[qmessage.mprocess.id] = qmessage.mprocess.memsize;
                  //fprintf(M_log, "#At time %d allocated %d bytes for process %d from %d to %d \n",getClk(),qmessage.mprocess.memsize,qmessage.mprocess.id,x,x+qmessage.mprocess.memsize-1);
                  push(PrQueueP, qmessage.mprocess, qmessage.mprocess.priority);  
                }
                else
                {   
                    push(Not_Alloc, qmessage.mprocess, qmessage.mprocess.memsize);
                    printf("Memory allocation failed\n");
                    fflush(stdout);
                }
            }
            if (currentlyrunning == 0 && PrQueueP->size > 0)
            {
                HPF(PrQueueP);
                currentlyrunning = 1;
                receiveDetails(0, log);
            }
        }
        break;
    case 2:
        // SRTN
        PriorityQueue *QueueRem = (PriorityQueue *)malloc(sizeof(PriorityQueue));
        QueueRem->size = 0;
        Process running;
        // QUEUE FROM PCBLOCK TO PRIORITY QUEUE
        while (true)
        {
            while (msgrcv(qId, &qmessage, sizeof(qmessage.mprocess), 0, !IPC_NOWAIT) != -1 && qmessage.mprocess.id != -1)
            {
                printf("Receiving %d\n", qmessage.mprocess.id);
                fflush(stdout);
                createprocess(qmessage.mprocess, PCBlock, processIndex);
                processIndex++;
                printf("SRTN  -  trying to allocate Process : %d with size :%d\n",qmessage.mprocess.id,qmessage.mprocess.memsize);
                fflush(stdout);
                int x = allocate(M_log,qmessage.mprocess,buddy, qmessage.mprocess.memsize);
                if (x != -1)
                {
                  printBuddySystem(buddy);
                  map[qmessage.mprocess.id] = x;
                  map_mem[qmessage.mprocess.id] = qmessage.mprocess.memsize;
                  //fprintf(M_log, "#At time %d allocated %d bytes for process %d from %d to %d \n",getClk(),qmessage.mprocess.memsize,qmessage.mprocess.id,x,x+qmessage.mprocess.memsize-1);
                 push(QueueRem, qmessage.mprocess, qmessage.mprocess.remainingTime); 
                }
                else
                {   
                    push(Not_Alloc, qmessage.mprocess, qmessage.mprocess.memsize);
                    printf("Memory allocation failed\n");
                    fflush(stdout);
                }
            }
            if (currentlyrunning == 1)
            {
                running.remainingTime --;
                if (receiveFinished(log,M_log,buddy,map,Not_Alloc,schedulerAlgo) == 0)
                {
                    currentlyrunning = 0;
                    cc++;
                    if (cc == noProcesses)
                    {
                        Writeperf();
                        msgctl(qId, IPC_RMID, NULL);
                        msgctl(detailsId, IPC_RMID, NULL);
                        msgctl(finishedId, IPC_RMID, NULL);
                        msgctl(runId, IPC_RMID, NULL);
                        printf("Scheduler is terminating\n");
                        fflush(stdout);
                        execl("killall process.out", "killall process.out", NULL);
                        destroyClk(true);

                        kill(0, SIGTERM);
                        exit(EXIT_SUCCESS);
                    }
                    if (currentlyrunning == 0 && QueueRem->size > 0)
                    {
                        running =SRTN(QueueRem);
                        currentlyrunning = 1;
                        receiveDetails(0, log);
                    }
                }
                else
                {
                    //printf(" RRunning id %d \n", running.id);

                    push(QueueRem, running, running.remainingTime);

                    Process p = peek(QueueRem);
                    if (p.id == running.id )
                    {
                        //SRTN(QueueRem);
                        //receiveDetails(5, log);
                        pop(QueueRem);
                    }
                    else
                    {
                        //printf("preemption\n");
                        SRTNback(QueueRem,running);
                        receiveDetails(1, log);
                        //printf("a \n");
                        SRTN(QueueRem);
                        //printf(" running id = %d and remaining time =%d \n", running.id,running.remainingTime);
                        //printf(" P id %d  and remaining time =%d  \n", p.id,p.remainingTime);
                        running = p;
                        //printf(" running id %d  and remaining time =%d  \n", running.id,running.remainingTime);

                        receiveDetails(0, log);
                        //printf("c \n");
                        
                    }
                }
            }

            
            if (currentlyrunning == 0 && QueueRem->size > 0)
            {
                running = SRTN(QueueRem);
                currentlyrunning = 1;
                receiveDetails(0, log);
            }
        }

        break;

    case 3:

        // RoundRobin
        int addIndex = 0;
        int deleteIndex = 0;
        int counter = q;
        int remProcessC = noProcesses;
        int timer = getClk();
        QRR = InstQueue();
        // QRR->size=noProcesses;

        // QUEUE FROM PCBLOCK TO PRIORITY QUEUE
        while (true)
        {
            while (msgrcv(qId, &qmessage, sizeof(qmessage.mprocess), 0, !IPC_NOWAIT) != -1 && qmessage.mprocess.id != -1)
            {

                createprocess(qmessage.mprocess, PCBlock, processIndex);
                processIndex++;

                printf("RR  -  trying to allocate Process : %d with size :%d\n",qmessage.mprocess.id,qmessage.mprocess.memsize);
                fflush(stdout);
                int x = allocate(M_log,qmessage.mprocess,buddy, qmessage.mprocess.memsize);
                if (x != -1)
                {
                  printBuddySystem(buddy);
                  //fprintf(M_log, "#At time %d allocated %d bytes for process %d from %d to %d \n",getClk(),qmessage.mprocess.memsize,qmessage.mprocess.id,x,x+qmessage.mprocess.memsize-1);
                  map[qmessage.mprocess.id] = x;
                map_mem[qmessage.mprocess.id] = qmessage.mprocess.memsize;
                  pushq(QRR, qmessage.mprocess);  
                }

                else
                {   
                    push(Not_Alloc, qmessage.mprocess, qmessage.mprocess.memsize);
                    printf("Memory allocation failed\n");
                    fflush(stdout);
                }
            } 
            timer++;
            if (currentlyrunning == 1)
            {
                counter--;
            }
            if (currentlyrunning == 1)
            {
                if (receiveFinished(log,M_log,buddy,map,Not_Alloc,schedulerAlgo) == 0)
                {

                    counter = q;
                    currentlyrunning = 0;
                    removeAtIndex(QRR, deleteIndex);
                    remProcessC--;
                    if (remProcessC == 0)
                    {
                        //
                    Writeperf();
                    msgctl(qId, IPC_RMID, NULL);
                    msgctl(detailsId, IPC_RMID, NULL);
                    msgctl(finishedId, IPC_RMID, NULL);
                    msgctl(runId, IPC_RMID, NULL);
                    printf("Scheduler is terminating\n");
                    fflush(stdout);
                    execl("killall process.out","killall process.out", NULL);
                    destroyClk(true);
                    
                    kill(0, SIGTERM);
                    exit(EXIT_SUCCESS);
                    //execl("killall scheduler.out","killall scheduler.out", NULL);
                        
                        
                        
                        //
                        break;
                    }

                    if (QRR->size == 0)
                    {
                        deleteIndex = -1;
                    }
                    else
                    {
                        deleteIndex--;
                    }
                }
            }

            if (counter == 0)
            {
                currentlyrunning = 0;
                RRback(QRR, deleteIndex);
                receiveDetails(1, log);
                counter = q;
            }

            if (currentlyrunning == 0 && QRR->size > 0)
            {

                deleteIndex = (deleteIndex + 1) % QRR->size;
                RR(QRR, deleteIndex);
                currentlyrunning = 1;
                receiveDetails(0, log);
            }
        }
        break;
    }

    // upon termination release the clock resources.
    // Delete message queues

    destroyClk(true);
}
