#include "Child.h"
#include "Directory.h"
void WriteTime(struct timespec start,struct timespec end,int partition);
void CheckInput(int argc,char* argv[],int* Grade,int* Processes,int* Requests,char** file);
void CheckPartitioning(int* partitions,int grade,char* filename);
void CreateProcesses(int argc,char* argv[]);
void FindPartition(int part,int partitions,int grade,char* filename,char* shm,sem_t** shmsem,int Processes,struct timespec* start);
sem_t** CreateSemaphores(sem_t** Semaphore1,sem_t** Semaphore2,sem_t** Semaphore3,sem_t** Semaphore4,int Processes);
void Cleanup(sem_t* Semaphore1,sem_t* Semaphore2,sem_t* Semaphore3,sem_t* Semaphore4,sem_t** shmsem,char* shm,int shmid,int Processes);
char* CreateSharedMemory(int* shmid,int grade);