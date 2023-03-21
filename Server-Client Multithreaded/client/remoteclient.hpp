#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <stdint.h>
#include <dirent.h>
#include <string>
#include <string.h>
#include <vector>
#include <fcntl.h>
#include <signal.h>
#define ARGS_ERROR -1
#define SOCKET_ERROR 1
#define CONNECTION_ERROR 2
#define READ_ERROR 3
#define DIRECTORY_ERROR 4
using namespace std;
void GetArgs(int argc,char** argv,char** ServerIP,int* port,char** Directory);
void ConnectToServer(char* ServerIP,int port,char* Directory);