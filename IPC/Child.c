#include "Child.h"

void ReadFromFile(int writer,int reader,sem_t* ChangeSharedMemory,sem_t* SharedMemoryAcesss,sem_t* WaitForChildrenInitialization,sem_t*  UseTempFile,sem_t* PostParentSemaphore)
{
    srand(getpid() * time(NULL)); //different value each time
    int bytes,fileinfo[3],read2,shmid,counter = 0,Request[2];
    char* shm;
    struct timespec start;
    bytes = read(reader,&fileinfo,sizeof(int)*3); ///read the partitioning and grading of the file
    if(bytes < 0) //error check
    {
        perror("failed to read");
        exit(READ_ERROR);
    }
    int outdesc = open("temp",O_APPEND|O_WRONLY,0644);
    if(outdesc < 0)
    {
        perror("Failed to open");
        exit(-1);
    }
    OpenSharedMemory(&shmid,&shm,fileinfo[1]);
    while(counter < fileinfo[2])
    {
        if(counter > 1) //check if a request has been written to the logfile
        {
            if(rand75() != true) //return true with a 75% probability
            { //if it returns false
                Request[0] = rand() % fileinfo[0] + 1; //find a new partition
                Request[1] = rand() % fileinfo[1] + 1; //and new line
            }
        }
        else 
        {//if the counter is  not greater than 1(process will request a segment for the first time)
            Request[0] = rand() % fileinfo[0] + 1; //get a value in range of 1 and the number of partitions
            Request[1] = rand() % fileinfo[1] + 1;  //get a value in range of 1 and the number of lines each partition has
        }
        sem_wait(UseTempFile); //wait if other processes are writing a request
            bytes = write(outdesc,&Request[0],sizeof(int)); //send to the parent the request
        sem_post(UseTempFile); //post so others can write a request
        clock_gettime(CLOCK_MONOTONIC_RAW,&start);
        if(bytes < 0) //error checking
        {
            perror("Failed to send desired file segments to parent");
            exit(PROCESS_ERROR);
        }
        if (counter == 0) //we want to post this only once in the beggining of the loop
            sem_post(WaitForChildrenInitialization);
        while(1) //how to fix this //wait to read from shared memory
        {
            sem_wait(SharedMemoryAcesss);
            bytes = read(reader,&read2,sizeof(int));
            if(bytes < 0)
            {
                perror("Failed to read from parent");
                exit(READ_ERROR);
            }
            if(read2 == Request[0])
                break;
        }
        WriteLog(shm,Request[1],Request[0],ChangeSharedMemory,PostParentSemaphore,&counter,start);
    }
    close(outdesc);
    CloseSharedMemory(shm);
}

void CloseSharedMemory(char* shm)
{
    if(shmdt(shm)<0) //detach from shared memory
    {
        perror("Child failed to detach from shared memory");
        exit(SHM_ERROR);
    }
}

void OpenSharedMemory(int* shmid,char** shm,int grade)
{
    key_t key = 5678; //a unique key for the shared memory
    *shmid = shmget(key,MAX_BUFFER_SIZE*grade,0666); //allocate enough space and set appropriate commands
    if(*shmid < 0) //error checkgrade
    {
        perror("Failed to locate shared memory segment");
        exit(SHM_ERROR);
    }
    *shm = shmat(*shmid,NULL,0); //attach to shared memorys
    if(*shm == (char*) - 1) //error check
    {
        perror("Shmat failed");
        exit(SHM_ERROR);
    }
}

void WriteLog(char* shm,int line,int part,sem_t* ChangeSharedMemory,sem_t* PostParentSemaphore,int* counter2,struct timespec start)
{
    int bytes,counter = 1;struct timespec end;
    char* logfile = (char*)malloc(100); //allocate space for a filename
    char* temp = (char*)malloc(100);
    strcpy(logfile,"./logfiles/");
    sprintf(temp,"%d",getpid()); //create a unique filename
    strcat(logfile,temp);
    free(temp);
    strcat(logfile,".log"); //add a .log to the end
    int fd = open(logfile,O_CREAT|O_APPEND|O_WRONLY,0644); //create a file
    if(fd < 0) //error check
    {
        perror("Failed to open log file");
        exit(LOG_ERROR);
    }    
    char* token = strtok(shm,"\n"); //if we want to get a line from this partition
    while(token != NULL) //check until we have no more lines
    { //increment counter each time we pass a line
        if(counter == line) //if we have found the requested line
        {
            clock_gettime(CLOCK_MONOTONIC_RAW, &end);
            long double final = ((long double)((end.tv_sec - start.tv_sec) * 1000000 + (end.tv_nsec - start.tv_nsec) / 1000)/1000000); //get the time in ms
            /*Here we pass in the time and the part and line of the file we read*/
            char* time = (char*)malloc(8); //create a string to pass it in to
            char* temp = (char*)malloc(4); //create a string to pass it in to
            char* etc = (char*)malloc(100);
            sprintf(temp,"%d",part); 
            sprintf(time,"%Lf",final); //convert the long double to a string
            strcpy(etc," ");
            strcat(etc,"<");strcat(etc,temp);strcat(etc,",");
            memset(temp,0,4);sprintf(temp,"%d",line);
            strcat(etc,temp); strcat(etc,">");strcat(etc," ");
            strcat(etc,time);strcat(etc," ms \n");
            /*From here on we write on to the file*/
            (*counter2)++; //update the second counter to know we have written one item to the logfile
            char* writer = (char*)malloc(MAX_BUFFER_SIZE + strlen(etc)); //allocate space for a write to write our line and time
            strcpy(writer,token);
            strcat(writer,etc);
            bytes = write(fd,writer,strlen(writer)); //write the line to the file
            if(bytes < 0) //error check
            {
                perror("Failed to write to log");
                exit(LOG_ERROR);
            }
            free(temp);
            free(etc);
            free(writer);
            free(time);
            close(fd); //close the file descriptor
            free(logfile); //free the variable
                break; //break from the loop
        }
        counter++;
        token = strtok(NULL,"\n"); //check the next line
    }
    usleep(20000); //wait 20ms after completion
    sem_wait(PostParentSemaphore);
        sem_post(ChangeSharedMemory); //post parent semaphore so he can begin to get the next partition
    sem_post(PostParentSemaphore);
}

//Probability Functions

int rand50() 
{//Return 0 or 1 with 50% probability for each
    return rand() & 1;
}

bool rand75()
{//Return 1 with 75% probability and 0 with 25%
    return !(rand50() & rand50());
}