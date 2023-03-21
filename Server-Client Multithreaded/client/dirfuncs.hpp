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
#define ARGS_ERROR -1
#define SOCKET_ERROR 1
#define CONNECTION_ERROR 2
#define READ_ERROR 3
#define DIRECTORY_ERROR 4
using namespace std;
void CreateDirectory(char* Path,int socket);
void RemoveDirItems(char* Directory);
void GetDirSize(const char* Directory,int& Counter);
