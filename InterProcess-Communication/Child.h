#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <signal.h>
#include <time.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <sys/wait.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/shm.h>
#include <sys/mman.h>
#include <stdbool.h>
#include <dirent.h>
#define MAX_BUFFER_SIZE 64*1024
#define READ 0
#define WRITE 1
#define INPUT_ERROR 2
#define FILE_ERROR 3
#define PROCESS_ERROR 4
#define PIPE_ERROR 5
#define PROCESS_TERMINATION 6
#define READ_ERROR 7
#define SEMAPHORE_ERROR 8
#define SHM_ERROR 9
#define LOG_ERROR 10
#define WRITE_ERROR 11
#define DIRECTORY_ERROR 12
int rand50();
bool rand75();
void OpenSharedMemory(int* shmid,char** shm,int grade);
void CloseSharedMemory(char* shm);
void WriteLog(char* shm,int line,int part,sem_t* semaphore,sem_t* sem2,int* counter2,struct timespec start);
void ReadFromFile(int writer,int reader,sem_t* semaphore,sem_t* shmsem,sem_t* semPar,sem_t* semChecker,sem_t* sem2);
