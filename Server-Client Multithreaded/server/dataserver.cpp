#include "dataserver.hpp"
#include "threads.hpp"

extern vector <pthread_t> WorkerThreads;
extern vector <pthread_t> Commthreads;
extern queue <DirVals> Queue;
extern pthread_mutex_t mutexQueue;
extern sem_t QueueFull;
extern sem_t QueueEmpty;

int main(int argc,char** argv)
{
    if(argc != 9)
    {
        if(argc > 9)
            cout<<"Too many arguments were given!"<<endl;
        else
            cout<<"Server needs more arguments to be initialized correctly!"<<endl;
        exit(ARGS_ERROR);
    }
    int port,threadpool,queuesize,blocksize; //variables to hold the values inserted from the command line
    GetArguments(argc,argv,&port,&threadpool,&queuesize,&blocksize); //this function gets the correct values from the one given in the command line
    pthread_mutex_init(&mutexQueue,NULL); //Initialiaze a mutex to handle the shared Global variable Queue
    sem_init(&QueueEmpty,0,queuesize); //initialize the semaphrore which notifies if there is space in the queue
    sem_init(&QueueFull,0,0); //initialize the semaphore which notifies if there is anything in the Queue
    CreateServer(port,threadpool,queuesize,blocksize); //create the server with the appropriate values
    pthread_mutex_destroy(&mutexQueue); //free the memory space used by the mutex
    sem_destroy(&QueueEmpty); //free the memory space used my the semaphores
    sem_destroy(&QueueFull);
    return 0;
}

void GetArguments(int argc,char** argv,int* port,int* threadpool,int* queuesize,int* blocksize)
{
    bool portnum = false,pool = false,queue = false,block = false; //booleans to mark if we found the correspoding value
    for(int i = 0;i < argc ; i++)
    {
        if(portnum == true) //if -p parameter was used
        {
            *port = atoi(argv[i]); //get the port ID
            portnum = false; //false so the if condition isn't satisfied at the next iteration
        }
        else if(pool == true) //if the -s param was used
        {
            *threadpool = atoi(argv[i]); //get the threadpool size
            pool = false;//false so the if condition isn't satisfied at the next iteration
        }
        else if(queue == true) //if the -q param was used
        {
            *queuesize = atoi(argv[i]); //get the queue size
            queue = false; //false so the if condition isn't satisfied at the next iteration
        }
        else if(block == true) //if the -b param was used
        {
            *blocksize = atoi(argv[i]); //get the block size
            block = false; //false so the if condition isn't satisfied at the next iteration
        }
        else if(strcmp(argv[i],"-p") == 0) //check if -p is in args
            portnum = true;
        else if(strcmp(argv[i],"-s") == 0) //check if -s is in args
            pool = true;
        else if(strcmp(argv[i],"-q") == 0) //check if -q is in args
            queue = true;
        else if(strcmp(argv[i],"-b") == 0) //check if -b is in args
            block = true;
    }
}

void CreateServer(int port,int threadpool,int queuesize,int blocksize)
{

    int sockfd,new_socket;
    struct sockaddr_in server;
    sockfd = socket(AF_INET,SOCK_STREAM,0); //create a socket using IPV4 addresses and a TCP connection
    if(sockfd < 0) //if it failes to be created
    {
        perror("Socket failed to create!");
        exit(SOCKET_ERROR);
    }
    server.sin_family = AF_INET; //set the server with appropriate values
    server.sin_addr.s_addr = htonl(INADDR_ANY); //any IP can connect to the server
    server.sin_port = htons(port);
    int opt = 1;
    int sockopt = setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,(char*)&opt,sizeof(opt));
    if(sockopt == -1)  //waht need to happen here?
    {
        perror("setsockopt failed ");
        exit(SOCKET_ERROR);
    }
    int binder = bind(sockfd,(struct sockaddr *)&server,sizeof(server)); //bind the socket
    if(binder == -1) //if binding fails
    {
        perror("Binding Failed!");  //error message
        exit(SOCKET_ERROR);   //exit error status
    }
    int listener = listen(sockfd,5);
    if(listener == -1) //check for connections
    {
        perror("Listen failed!"); //error message
        exit(SOCKET_ERROR); //exit error status
    }
    else
        cout<<"Server Successfully initialized..."<<endl; // if listen was successfully called,print a success message
    struct sockaddr_in client; //client struct
    socklen_t clientlen = sizeof(client); //get the size of the struct
    CreateWorkerThreads(threadpool,blocksize); //vector to hold all the worker threads
    while(1) //an infinite loop since we consider the server to run indefinitely
    {
        new_socket = accept(sockfd,(struct sockaddr *)&client,&clientlen); //wait to accept the client requesting to connect to the server
        if(new_socket == -1) //check for errors
        {
            perror("Server failed to accept client "); //error message
            exit(SERVER_ERROR); //exit error status
        }
        else
            cout<<"Server accepted client with address : "<<client.sin_addr.s_addr<<endl; //print connection acceptane message
        int bytessent = send(new_socket,(char*)&blocksize,sizeof(int),0); //send the Blocksize to the client(this is a part of the client-server protocol implemented)
        if(bytessent == -1)  //check for errors
        {
            perror("Failed to send packet size to client"); //error message
            exit(SOCKET_ERROR); //exit error status
        }
        int* new_sockptr = new int;
        *new_sockptr = new_socket;
        pthread_t Commthread; //initialize a thread val
        Commthreads.push_back(Commthread); //add it to the vector
        if(pthread_create(&Commthreads.back(),NULL,Communication,new_sockptr) != 0) //create thread and give it a task
        {
            perror("Failed to create thread"); //error message
            exit(COMMTHREAD_ERROR); //exit error status
        }
        cout<<"Thread with ID: "<<Commthreads.back()<<" was created!"<<endl; //print a successful creation message
        if(pthread_detach(Commthreads.back()) != 0)
        {
            perror("Failed to join thread");
            exit(COMMTHREAD_ERROR);
        }
    }
    close(sockfd); //close the server socket
}