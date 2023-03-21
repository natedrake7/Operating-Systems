#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <stdint.h>
#include <dirent.h>
#include <fcntl.h>
#include <string>
#include <string.h>
#include <vector>
#include <queue>
#include <semaphore.h>
#define ARGS_ERROR -1
#define SOCKET_ERROR 1
#define SERVER_ERROR 2
#define COMMTHREAD_ERROR 3
#define DIRECTORY_ERROR 4
#define MUTEX_ERROR 5
#define WORKER_THREAD_ERROR 6
using namespace std;

void GetArguments(int argc,char** argv,int* port,int* threadpool,int* queuesize,int* blocksize);
void CreateServer(int port,int threadpool,int queuesize,int blocksize);