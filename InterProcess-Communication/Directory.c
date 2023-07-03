#include "Directory.h"

void CreateDirectory()
{
    int check = DeleteDirectory("./logfiles"); //check if the directory exists 
    if(check == 0) //if it existed and is now empty
    {
        if(rmdir("./logfiles") < 0) //delete it
        {
            perror("Failed to delete logfiles directory"); //error checking
            exit(DIRECTORY_ERROR);
        }
    }
    if(mkdir("./logfiles",0777) < 0) //create it again
    {
        perror("Failed to create logfiles directory"); //error checking
        exit(DIRECTORY_ERROR);
    }
}

int DeleteDirectory(const char* dirname)
{
    DIR* dir = opendir(dirname); //open the directory
    if(dir == NULL) //if it is null ,it means the directory doesnt exist
        return 1;
    
    struct dirent* entity;
    entity = readdir(dir); //get the contents of the directory
    char* path = (char*)malloc(100);
    while(entity != NULL) //if it not null(the directory is not empty)
    {
        if(strcmp(entity->d_name,".") != 0 && strcmp(entity->d_name,"..") != 0) //if the file in the directory is not another directory(there shouldnt be,but there are the ./ and ../ directories)
        { //so we avoid them
            strcat(path,dirname);//create the full path of the file
            strcat(path,"/"); //since remove needs the full path
            strcat(path,entity->d_name);
            if(remove(path) < 0) //remove the logfile
            {
                perror("Failed to delete logfile"); //error checking
                exit(DIRECTORY_ERROR);
            }
            memset(path,0,100); //allocate space for path variable to be used again
        }
        entity = readdir(dir); //proceed to next file in current directory
    }
    free(path); //free path
    closedir(dir); //close the directory pointer
    return 0; //return 0 so we now there were items in the directory
}

int CreateTemporaryFile()
{ //This is a file created that children will write all their requests
    int fd = open("temp",O_CREAT|O_EXCL|O_APPEND|O_RDONLY,0644); //Create a temporary file for all children to write to
    if(fd < 0) //error checking
    {
        if(errno != EEXIST)//if the error is not that is already exists
        {
            perror("Failed to open read file");
            exit(-1);
        }//else if the file exists
        if(remove("temp") < 0) //remove it
        {
            perror("Failed to delete temporary file"); //error checking
            exit(-1);
        }
        fd = open("temp",O_CREAT|O_EXCL|O_APPEND|O_RDONLY,0644); //try to open it again
        if(fd < 0) //error check again to be sure
        {
            perror("Failed again to create temporary file");
            exit(-1);
        }
    }
    return fd;
}

void CreateParentLog()
{
    int fd = open("./logfiles/parent.log",O_CREAT|O_EXCL|O_APPEND|O_WRONLY,0644);
    if(fd < 0)
    {
        if(errno != EEXIST)
        {
            perror("Failed to open parent log");
            exit(-1);
        }
        if(remove("./logfiles/parent.log") == -1)
        {
            perror("Failed to remove parent log from previous run");
            exit(-1);
        }
        fd = open("./logfiles/parent.log",O_CREAT|O_EXCL|O_APPEND|O_WRONLY,0644); //try to open it again
        if(fd < 0) //error check again to be sure
        {
            perror("Failed again to create temporary file");
            exit(-1);
        }
    }
    close (fd);
}