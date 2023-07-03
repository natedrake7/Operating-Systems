#include "Parent.h"

int main(int argc,char* argv[])
{
    CreateProcesses(argc,argv); //create its children
    return 0;
}

void CheckInput(int argc,char* argv[],int* Grade,int* Processes,int* Requests,char** file)
{
    bool text = false,grading = false,process = false,request = false;
    if(argc!=9)
    {
        printf("You used incorrect input!\n");
        printf("For example an execution looks like : ./test -t 100.txt -p 10 -r 100 -g 10\n");
        exit(INPUT_ERROR);
    }
    for(int i = 0; i < argc;i++)
    {
        if(text == true)
        {
            *file = argv[i];
            text = false;
        }
        else if(process == true)
        {
            *Processes = atoi(argv[i]);
            process = false;
        }
        else if(request == true)
        {
            *Requests = atoi(argv[i]);
            request = false;
        }
        else if(grading == true)
        {
            *Grade = atoi(argv[i]);
            grading = false;
        }        
        if(strcmp(argv[i],"-t") == 0)
            text = true;
        if(strcmp(argv[i],"-p") == 0)
            process = true;
        if(strcmp(argv[i],"-r") == 0)
            request = true;
        if(strcmp(argv[i],"-g") == 0)
            grading = true;
    }
}

void CheckPartitioning(int* partitions,int grade,char* filename)
{
    int fd = open(filename,O_RDONLY,0644);
    if(fd < 0)
    {
        perror("Failed to open requested file");
        exit(FILE_ERROR);
    }
    char* reader = (char*)malloc(MAX_BUFFER_SIZE);
    int bytes,counter = 1;
    while(1)
    {
        char* token;
        bytes = read(fd,reader,MAX_BUFFER_SIZE);
        if(bytes < 0)
        {
            perror("Failed to read from file");
            exit(READ_ERROR);
        }
        if(bytes == 0)
            break;
        token = strtok(reader,"\n");
        while(token != NULL)
        {
            counter++;
            token = strtok(NULL,"\n");
        }
    }
    free(reader);
    if(counter < 1000)
    {
        printf("You need to input a bigger file!\n");
        exit(FILE_ERROR);
    }
    if(counter % grade  > 0) //if there is a module from the division of the number of lines and the grading(how many lines each partition has) then we have one more extra partition
        *partitions = ( counter / grade ) + 1;
    else //the division has no module
        *partitions = counter/ grade;
}


void CreateProcesses(int argc,char* argv[])
{
    char* filename;
    int fileinfo[3],Processes; //hold the partitioning and grading
    CreateDirectory();
    CheckInput(argc,argv,&fileinfo[1],&Processes,&fileinfo[2],&filename); //get the filename partitioning and grading of the file
    CheckPartitioning(&fileinfo[0],fileinfo[1],filename);
    pid_t pids[Processes]; //an array for the child processes
    int writepipes[Processes][2],readpipes[Processes][2]; //writepipes are used for the children to write to the parent process(so the parent process reads through them)
    //and readpipes are used for the children to read from the parent process(so the parent process writes through them)
    for(int i=0;i<Processes;i++)//Initialize all the pipes
    {
        if(pipe(writepipes[i]) == -1) //Error checking
        {
            perror("failed to create pipe");
            exit(PIPE_ERROR);
        }
        if(pipe(readpipes[i]) == -1) //for both the arrays of pipes
        {
            perror("failed to create pipe");
            exit(PIPE_ERROR);
        }
    }
    sem_t* ChangeSharedMemory;sem_t* WaitForChildrenInitialization;sem_t* UseTempFile;sem_t* PostParentSemaphore;
    sem_t** SharedMemoryAcesss = CreateSemaphores(&ChangeSharedMemory,&WaitForChildrenInitialization,&UseTempFile,&PostParentSemaphore,Processes);
    for(int i = 0;i < Processes; i++) //a for loop to initialize the children
    {
        pids[i] = fork(); //fork for each child
        if(pids[i] == -1) //error checking
        {
            perror("Failed to create process");
            exit(PROCESS_ERROR);
        }
        else if(pids[i] == 0) //child process
        {
            for(int j=0;j<Processes;j++) //a loop to close all descriptors of the pipes we won't be using in this process
            {
                if(i != j) //each process's position in the pid array is the position of its pipes in the pipes array
                {
                    close(readpipes[j][READ]);
                    close(readpipes[j][WRITE]);
                    close(writepipes[j][READ]);
                    close(writepipes[j][WRITE]);
                }
                else //so they must be equal
                {
                    close(readpipes[j][WRITE]); //close the write end because we"ll only read from it
                    close(writepipes[j][READ]); //close the read end because we"ll write from it
                }
            }
            kill(getpid(),SIGSTOP); //wait for parent to continue this process
            ReadFromFile(writepipes[i][WRITE],readpipes[i][READ],ChangeSharedMemory,SharedMemoryAcesss[i],WaitForChildrenInitialization,UseTempFile,PostParentSemaphore);
            close(readpipes[i][READ]); //close the remaining pipes
            close(writepipes[i][WRITE]);
            printf("Process %d finished!\n",getpid());
            exit(PROCESS_TERMINATION); //exit and terminate
        }
    }
    //here is the main process only
    int bytes; //each partition has a total number of grade lines
    for(int i=0;i<Processes;i++) //each iteration send to a child process
    {
        bytes = write(readpipes[i][WRITE],&fileinfo,sizeof(int)*3); //write to the children the grading and partitionings
        if(bytes < 0) //error checking
        {
            perror("failed to write");
            exit(INPUT_ERROR);
        }
    }
    int shmid,partition,temp,i = 0;
    int fd = CreateTemporaryFile(); //Create a temporary file (see the function for more info)
    CreateParentLog();
    char* shm = CreateSharedMemory(&shmid,fileinfo[1]);
    struct timespec start,end;
    kill(-1,SIGCONT); //send message to the children to continue their execution
    sem_wait(WaitForChildrenInitialization);
    while(1)
    {
        sem_wait(ChangeSharedMemory); //wait for all children using it to be done so another can be inserted
        sem_wait(UseTempFile);
            bytes = read(fd,&partition,sizeof(int)); //read the partition and line selected from the ith child;
        sem_post(UseTempFile);
        if(bytes < 0) //error checking
        {
            perror("Failed to read from child");
            exit(READ_ERROR);
        }
        else if(bytes == 0)
        {
            for(int i = 0; i < Processes;i++)
                wait(NULL); //to finish
            printf("No more input!\nExiting!\n");
            break;
        }
        for(int i = 0;i < Processes;i++)
        {
            if(write(readpipes[i][WRITE],&partition,sizeof(int)) < 0)
            {
                perror("Failed to write to child");
                exit(READ_ERROR);
            }
        }
        /*Here is when the file final exits the shared memory*/
        memset(shm,0,MAX_BUFFER_SIZE*fileinfo[1]); //empty the shared memory
        if(i > 0)
            WriteTime(start,end,temp);
        FindPartition(partition,fileinfo[0],fileinfo[1],filename,shm,SharedMemoryAcesss,Processes,&start); //find the requested partitions
        temp = partition;
        i++;
    }
    close(fd);
    Cleanup(ChangeSharedMemory,WaitForChildrenInitialization,UseTempFile,PostParentSemaphore,SharedMemoryAcesss,shm,shmid,Processes);
}

void WriteTime(struct timespec start,struct timespec end,int partition)
{
    char* writer = (char*)malloc(100);
    char* temp = (char*)malloc(100);
    clock_gettime(CLOCK_MONOTONIC_RAW,&end);
    int fd = open("./logfiles/parent.log",O_APPEND|O_WRONLY,0644); //Create a temporary file for all children to write to
    if(fd < 0) //error checking
    {
        perror("Failed to open parent log");
        exit(-1);
    }
    long double final = ((long double)((end.tv_sec - start.tv_sec) * 1000000 + (end.tv_nsec - start.tv_nsec) / 1000)/1000000); //get the time in ms
    sprintf(writer,"%d",partition);
    sprintf(temp,"%Lf",final);
    strcat(writer," ");
    strcat(writer,temp);
    strcat(writer,"ms \n");
    int bytes = write(fd,writer,strlen(writer));
    if(bytes < 0)
    {
        perror("Failed to write to parent log");
        exit(-1);
    }
    free(writer);
    free(temp);
    close(fd);
}

void Cleanup(sem_t* Semaphore1,sem_t* Semaphore2,sem_t* Semaphore3,sem_t* Semaphore4,sem_t** shmsem,char* shm,int shmid,int Processes)
{
    //destroy the file created
    if(remove("temp") == -1)
    {
        perror("Failed to delete temporary file");
        exit(-1);
    }
    //destroy the semaphores
    if(sem_destroy(Semaphore1) < 0) //error check
    {
        perror("Failed to close semaphore");
        exit(SEMAPHORE_ERROR);
    }
    munmap(Semaphore1, sizeof(Semaphore1)); //remove them from the shared memory
    if(sem_destroy(Semaphore2) < 0)
    {
        perror("Failed to close semaphore");
        exit(SEMAPHORE_ERROR);
    }
    munmap(Semaphore2, sizeof(Semaphore2)); //remove semaphores from shared memory
    if(sem_destroy(Semaphore3) < 0)
    {
        perror("Failed to close semaphore");
        exit(SEMAPHORE_ERROR);
    }
    munmap(Semaphore3, sizeof(Semaphore3)); //remove them from the shared memory
    if(sem_destroy(Semaphore4) < 0)
    {
        perror("Failed to close semaphore");
        exit(SEMAPHORE_ERROR);
    }
    munmap(Semaphore4, sizeof(Semaphore4));
    for(int i = 0;i < Processes;i++) //destroy and unmap each semaphore from the array
    {
       if(sem_destroy(shmsem[i]) < 0)
        {
            perror("Failed to close semaphore");
            exit(SEMAPHORE_ERROR);
        }
        munmap(shmsem[i], sizeof(shmsem[i]));
    }
    free(shmsem); //and free its memory
    if(shmdt(shm) < 0) //detach from shared memory
    {
        perror("Failed to detach from shared memory");
        exit(SHM_ERROR);
    }
    if(shmctl(shmid,IPC_RMID,NULL) < 0) //destroy shared memory
    {
        perror("Failed to destroy shared memory");
        exit(SHM_ERROR);
    }
}

char* CreateSharedMemory(int* shmid,int grade)
{
    key_t key = 5678;
    *shmid = shmget(key,MAX_BUFFER_SIZE*grade,IPC_CREAT | 0666); //get a shared memory segment
    if(*shmid < 0) //error check
    {
        perror("Failed to create shared memory");
        exit(SHM_ERROR);
    }
    char* shm = shmat(*shmid,NULL,0); //attach to shared memory segment
    if(shm == (char*) -1) //error checking
    {
        perror("Failed to attach segment to memory space");
        exit(SHM_ERROR);
    }
    return shm;
}

sem_t** CreateSemaphores(sem_t** Semaphore1,sem_t** Semaphore2,sem_t** Semaphore3,sem_t** Semaphore4,int Processes)
{
    //Create a set of semaphores each with a different job
    *Semaphore1 = mmap(0, sizeof(sem_t), PROT_READ|PROT_WRITE,
                  MAP_SHARED|MAP_ANONYMOUS, -1, 0);
    if(sem_init(*Semaphore1,1,1) < 0)
    {
        perror("failed to initialze semaphore");
        exit(SEMAPHORE_ERROR);
    }
    *Semaphore2 = mmap(0, sizeof(sem_t), PROT_READ|PROT_WRITE,
                  MAP_SHARED|MAP_ANONYMOUS, -1, 0); //work in progress
    if(sem_init(*Semaphore2,1,0) < 0)
    {
        perror("failed to initialze semaphore");
        exit(SEMAPHORE_ERROR);
    }
    *Semaphore3 = mmap(0, sizeof(sem_t), PROT_READ|PROT_WRITE,
                  MAP_SHARED|MAP_ANONYMOUS, -1, 0); //work in progress
    if(sem_init(*Semaphore3,1,1) < 0)
    {
        perror("failed to initialze semaphore");
        exit(SEMAPHORE_ERROR);
    }
    *Semaphore4 = mmap(0, sizeof(sem_t), PROT_READ|PROT_WRITE,
                  MAP_SHARED|MAP_ANONYMOUS, -1, 0); //work in progress
    if(sem_init(*Semaphore4,1,1) < 0)
    {
        perror("failed to initialze semaphore");
        exit(SEMAPHORE_ERROR);
    }
    sem_t** shmsem = malloc(Processes*sizeof(sem_t*)); //this is an array of semaphores each corresponding to one of our processes
    for(int i = 0;i < Processes;i++)
    {
        shmsem[i] = mmap(0, sizeof(sem_t), PROT_READ|PROT_WRITE,
                MAP_SHARED|MAP_ANONYMOUS, -1, 0); 
        if(sem_init(shmsem[i],1,0) < 0)
        {
            perror("failed to initialze semaphore");
            exit(SEMAPHORE_ERROR);
        }
    }
    return shmsem;
}


void FindPartition(int part,int partitions,int grade,char* filename,char* shm,sem_t** shmsem,int Processes,struct timespec* start)
{
    char* reader = (char*)malloc(MAX_BUFFER_SIZE); //a reader like before
    int counter = 1,bytes; //a counter and a variable to hold how many bytes are read
    int fd = open(filename,O_RDONLY); //open the file in read mode
    if(fd < 0) //error check
    {
        perror("Failed to open file");
        exit(FILE_ERROR);
    }
    while(1) //a loop that terminates if we find the partition we want or we reach EOF
    {
        bytes = read(fd,reader,MAX_BUFFER_SIZE); //read from the file
        if(bytes == -1) //error checking
        {
            perror("failed to read from file");
            exit(FILE_ERROR);
        }
        if(counter > part*grade) //if we surpass the partition we have break the loop again
            break;
        char* token = strtok(reader,"\n"); //break the reader string by the new line delimiter
        while(token!=NULL) //if the string has at least one delimiter
        {
            counter++; //increment the counter(counter hold how many lines deep in the file we are)
            if(counter > (part - 1)*grade && counter <= part*grade)
            {  
                //if we are above the last line of the previous parition and under the end of the requested partition
                strcat(shm,token); //add to the end of shm the token (the line)
                strcat(shm,"\n"); //add a new line so the partition remains the same
            }
            token = strtok(NULL,"\n"); //get the rest of the lines
        }
    }
    /*From Here the file has entered the shared memory*/
    clock_gettime(CLOCK_MONOTONIC_RAW,start);
    for(int i = 0;i <Processes;i++) //unlock all processes
        sem_post(shmsem[i]); //so they can check the shared memory
    free(reader); //free reader
    close(fd); //close the file
}