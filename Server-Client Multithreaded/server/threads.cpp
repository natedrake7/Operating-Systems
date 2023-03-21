#include "threads.hpp"

vector <pthread_t> WorkerThreads;
vector <pthread_t> Commthreads;
queue <DirVals> Queue;
pthread_mutex_t mutexQueue;
sem_t QueueFull;
sem_t QueueEmpty;


void CreateWorkerThreads(int ThreadPool,int BlockSize) //creates the worker threads according to the threadpool parameter
{
    int* blocksize = new int;
    *blocksize = BlockSize;
    for(int i = 0;i < ThreadPool;i++) //for loop
    {
        pthread_t Thread; //a new thread variable
        WorkerThreads.push_back(Thread); //push back the new thread
        if(pthread_create(&WorkerThreads.back(),NULL,task,blocksize) != 0) //if it failed to initialize
        {
            perror("Failed to create worker threads "); //error message
            exit(WORKER_THREAD_ERROR); //exit error status
        }
    }
}

void* task(void* args) //this is the job of the worker threads
{
    DirVals File; //a struct to hold the socket and the file directory
    int BlockSize = *(int*)args;
    int fd,bytes;
    int BytesSent;
    struct stat st;
    while(1)
    {
        sem_wait(&QueueFull); //wait for QueueFull to become greater than 1 should start at 0 so wait
        pthread_mutex_lock(&mutexQueue); //lock the mutex so we can use the global Queue
        File = Queue.front();
        Queue.pop();
        pthread_mutex_unlock(&mutexQueue); //unlock the mutex so other threads can use the queue
        sem_post(&QueueEmpty); //notify the other threads that the queue has one available slot,so the communcation threads can write to it
        bytes = send(File.socket,File.Directory,BlockSize,0);
        if(bytes == -1) //check if send was successful
        {
            if(errno != EBADF) //if errno is EBADF it means the client cut his connection with the server ,so the server should return an exit error status and continue to handle more clients
            {
                perror("Failed to send file name to client"); //error message
                exit(WORKER_THREAD_ERROR); //exit error status
            }
        }
       /* if(stat(File.Directory,&st) == 0)
        {  
            string temp = to_string(st.st_size); //use the string constructor to get the value of the size of the file , as a string since it is easier to send through sockets
            int TSize = st.st_size;
            cout<<temp.c_str()<<endl;
            bytes = write(*File.socket,temp.c_str(),temp.size());
            if(bytes == -1)
            {
                perror("Failed to send File Size to client");
                exit(WORKER_THREAD_ERROR);
            }
              /*  if(!S_ISDIR(st.st_mode))
                {
                    fd = open(File.Directory,O_RDONLY,0644); //try to open the current directory
                    if(fd == -1) //if there is an error
                    {
                        perror("Failed to open file");
                        exit(WORKER_THREAD_ERROR);
                    }
                    char* message = new char[*BlockSize];
                    while(1)
                    {
                        memset(message,0,*BlockSize);
                        bytes = read(fd,message,*BlockSize);
                        if(bytes == -1)
                        {
                            perror("Error sending file to client");
                            exit(WORKER_THREAD_ERROR);
                        }
                        if(bytes == 0)
                            break;
                        BytesSent = write(*File.socket,message,bytes);
                        if(BytesSent == -1)
                        {
                            perror("Failed to write file contents to client");
                            exit(WORKER_THREAD_ERROR);
                        }
                    }
                    close(fd);
                }
            }*/
    }
}

void* Communication(void* args)
{
    int bytesav;
    int SocketVal  = *(int*)args;
    int counter = 0;
    int bytesread = ioctl(*(int*)args,FIONREAD,&bytesav); //get available bytes from socket
    if(bytesread == -1)
    {
        perror("Ioctl failed"); //error handling
        exit(COMMTHREAD_ERROR); //exit error status
    }
    char* message = new char[bytesav]; //allocate memory to read the directory
    int bytes = read(*(int*)args,message,bytesav); //read the directory
    if(bytes == -1)
    {
        perror("Failed to read from client"); //error handling
        exit(COMMTHREAD_ERROR); //exit error status
    }
    //get the directory items recursively
    char* Dir = new char[strlen(message) + 3];
    strcpy(Dir,"./");
    strcat(Dir,message);
    GetDirItems(Dir,SocketVal,counter);
    ++counter;
    string temp = to_string(counter); //write the file count to the client so he knows when to stop reading  
    bytes = write(SocketVal,temp.c_str(),sizeof(int));
    if(bytes == -1)
    {
        perror("Failed to write file count to client");
        exit(COMMTHREAD_ERROR);
    }
    char* exitmessage = new char[100];
    bytes = recv(SocketVal,exitmessage,sizeof(exitmessage),0);
    if(bytes == -1)
    {
        perror("Failed to receive termination message from client");
        exit(COMMTHREAD_ERROR);
    }
    if(strcmp(exitmessage,"Done") == 0)
        close(SocketVal);
    free(Dir);
    free(message);
    free(exitmessage);
}

void GetDirItems(const char* Directory,int Socket,int& Counter)
{
    DIR* Dir = opendir(Directory); //open the directory given by the client
    DirVals File; //initialize a Queue Object
    if(Dir != NULL) //check if the given directory exists
    {
        struct dirent* SubDir; //get a subdir pointer
        SubDir = readdir(Dir); //readdir func
        while(SubDir != NULL) //while we dont reach the end of the directory
        {
            if(strcmp(SubDir->d_name,".")!=0 && strcmp(SubDir->d_name,"..")!=0) //avoid the current and upper directories
            {
                File.Directory = new char[4096]; //initialize the values in the struct of the object
                char* temp = new char[4096];
                strcpy(temp,Directory); //copy the directory path to a temp value
                strcat(temp,"/"); //add the /
                strcat(temp,SubDir->d_name); //and copy the subdir name so we have a complete path
                strcpy(File.Directory,temp); //finally copy that to the Queue object
                free(temp); //free the temp memory space
                File.socket = Socket; //and the current socket of the client
                sem_wait(&QueueEmpty); //at least one position in the queue is available
                pthread_mutex_lock(&mutexQueue); //lock the mutex so no other thread can use the queue
                Queue.push(File); //push the file
                pthread_mutex_unlock(&mutexQueue); //unlock the mutex so other threads(worker threads) can access it
                sem_post(&QueueFull); //notify that the queue has one more item
                Counter++;
                if(SubDir->d_type == DT_DIR) //if the subdir is a directory and not a file
                {
                    char* path = new char[4096]; //create a temp val
                    strcpy(path,Directory);
                    strcat(path,"/");
                    strcat(path,SubDir->d_name); //undergo the same process as above in order to have a complete directory path
                    GetDirItems(path,Socket,Counter); //call recursively
                    free(path); //free the path memory space
                }
            }
            SubDir = readdir(Dir); //if not call readdir again and go deeper in the directory
        }
    }
    else
        cout<<"The requested directory doesn't exist!"<<endl;
    closedir(Dir); //close the directory
}