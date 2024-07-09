#include "headers.h"
#include "process.h"

void clearResources(int);

int main(int argc, char *argv[])
{
    signal(SIGINT, clearResources);
    // TODO Initialization
    // 1. Read the input files.
    FILE *input;
    input = fopen("processes.txt", "r");
    if (input == NULL)
    {
        printf("Error in reading file.\n");
        fflush(stdout);
        return 0;
    }
    // ignoring first line.
    char firstLine[40];
    fgets(firstLine, sizeof(firstLine), input);

    // counting number of processes
    int count = 0;
    while (true)
    {
        if (fgets(firstLine, sizeof(firstLine), input) != NULL)
        {
            count++;
        }
        else
        {
            break;
        }
    }
    // return to the begining of input file
    fseek(input, 0, SEEK_SET);
    printf("count: %d\n", count);
    fflush(stdout);
    // ignoring first line.
    fgets(firstLine, sizeof(firstLine), input);

    int id, arrival, runTime, priority,memsize;
    Process *processesList = (Process *)malloc(count * sizeof(Process));
    for (int i = 0; i < count; i++)
    {
        fscanf(input, "%d", &id);
        fscanf(input, "%d", &arrival);
        fscanf(input, "%d", &runTime);
        fscanf(input, "%d", &priority);
        fscanf(input, "%d", &memsize);
        
        Process temp;
        constProcess(&temp, id, arrival, runTime, priority,memsize);
        processesList[i] = temp;
    }
    for(int i = 0 ; i < count ; i++)
    {
        printf("id: %d, arrival: %d, runTime: %d, priority: %d, memsize: %d\n",processesList[i].id,processesList[i].arrival,processesList[i].runTime,processesList[i].priority,processesList[i].memsize);
        fflush(stdout);
    }
    // 2. Ask the user for the chosen scheduling algorithm and its parameters, if there are any.
    char schedAlgo[5];
    char quantum[5];
    char countString[5];
    snprintf(countString, sizeof(countString), "%d", count);

    printf("Enter 1 for High Priority First, 2 for Shortest Remaining Time Next, 3 for Round-Robin:\n");
    fflush(stdout);
    scanf("%s", schedAlgo);
    if (atoi(schedAlgo) == 3)
    {
        printf("Enter quantum:\n");
        fflush(stdout);
        scanf("%s", quantum);
    }

    // 3. Initiate and create the scheduler and clock processes.
    int schedulerPID = fork();
    if (schedulerPID == -1)
    {
        // Handle fork failure
        perror("fork");
    }
    else if (schedulerPID == 0)
    {
        // Child process
        execl("./scheduler.out", "./scheduler.out", schedAlgo, quantum, countString, NULL);
    }

    int cPID = fork();
    if (cPID == 0)
    {
        execl("./clk.out", "./clk.out", NULL);
    }

    // 4. Use this function after creating the clock process to initialize clock
    initClk();

    // To get time use this
    int x = getClk();
    printf("current time is %d\n", x);
    fflush(stdout);

    // TODO Generation Main Loop
    // 5. Create a data structure for processes and provide it with its parameters.
    int currentProcess = 0;
    key_t key = (ftok("keyfile", 0));
    int qID = msgget(key, 0666 | IPC_CREAT);
    struct msgbuff message;
    int timeUpdate = getClk();
    
    while (true)
    {
        if (getClk() != timeUpdate)
        {
            timeUpdate++;
            while (getClk() == processesList[currentProcess].arrival)
            {
                message.mprocess = processesList[currentProcess];
                currentProcess++;
                message.mtype = 1;
                msgsnd(qID, &message, sizeof(message.mprocess), !IPC_NOWAIT);
            }
            message.mtype = 1;
            message.mprocess.id = -1;
            msgsnd(qID, &message, sizeof(message.mprocess), !IPC_NOWAIT);
        }
    }

    // 6. Send the information to the scheduler at the appropriate time.
    // 7. Clear clock resources
    // destroyClk(true);
}

void clearResources(int signum)
{
// TODO Clears all resources in case of interruption
printf("Interrupt signal received. Clearing resources...\n");
fflush(stdout);
// Clear clock resources
destroyClk(true);
execl("killall process.out","killall process.out", NULL);
execl("killall scheduler.out","killall scheduler.out", NULL);
execl("ipcrm -a","ipcrm -a", NULL);
// Terminate the program
exit(0);
}
