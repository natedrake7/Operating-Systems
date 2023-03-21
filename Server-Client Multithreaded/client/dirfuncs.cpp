#include "dirfuncs.hpp"

void CreateDirectory(char* Path,int socket)
{
    char* Token = strtok(Path,"/"); //break the path by /
    vector <char*> TokenArray;
    vector <char*>::iterator it;
    char* pathname = new char[4096];
    char* Dir = new char[4096];
    int fd;
    while(Token != NULL) //while the token is not null
    {
        if(strcmp(Token,".") != 0) //if it is the current dir we dont care
        {
                if(strcmp(Token,"..") != 0) //also if it is the upper directory
                {
                    TokenArray.push_back(Token); //add it to the end of the array
                    for(it = TokenArray.begin();it!=TokenArray.end();it++) //traverse the array
                    {
                        strcat(pathname,"/"); //else add a / to the end of pathname
                        strcat(pathname,*it); //and add the string of the token to pathname
                    }
                    strcpy(Dir,"./"); //copy the relative path to the Dir
                    strcat(Dir,pathname); //and add the pathname to the Dir
                   // RemoveDirItems(Dir);
                    if(strstr(pathname,".") == NULL) //if there is no. so it is not a regular file
                    {
                        if(mkdir(Dir,0777) == -1) //create the folder
                        {
                            if(errno != EEXIST) //the remove files  function was not implemented
                            {
                                perror("Failed to create desired directory ");
                                exit(DIRECTORY_ERROR);
                            }
                        }
                    }
                    else //else it is a regular file
                    {
                        fd = open(Dir,O_CREAT,0644); //create it with open
                        if(fd == -1)
                        {
                            perror("Failed to create file");
                            exit(DIRECTORY_ERROR);
                        }
                        close(fd); //close the descriptor
                    }
                    memset(Dir,0,4096); //allocate space to Dir
                }
        }
        memset(pathname,0,4096); //allocate space to pathname
        Token = strtok(NULL,"/"); //call strtok again
    }
    free(pathname); //free pathname
    free(Dir); //free Dir
}

void RemoveDirItems(char* Directory) //not working correctly
{
    int check;
    DIR* Dir = opendir(Directory); //open the directory given by the client
    if(Dir) //check if the given directory exists
    {
        check = remove(Directory);
        if(check == -1)
        {
            perror("Failed to remove directory");
            cout<<Directory<<endl;
            exit(DIRECTORY_ERROR);
        }
    }
    closedir(Dir); //close the directory
}

void GetDirSize(const char* Directory,int& Counter)
{
    DIR* Dir = opendir(Directory); //open the directory given by the client
    if(Dir != NULL) //check if the given directory exists
    {
        struct dirent* SubDir; //get a subdir pointer
        SubDir = readdir(Dir); //readdir func
        while(SubDir != NULL) //while we dont reach the end of the directory
        {
            if(strcmp(SubDir->d_name,".")!=0 && strcmp(SubDir->d_name,"..")!=0) //avoid the current and upper directories
            {
                char* temp = new char[4096];
                strcpy(temp,Directory); //copy the directory path to a temp value
                strcat(temp,"/"); //add the /
                strcat(temp,SubDir->d_name); //and copy the subdir name so we have a complete path
                free(temp); //free the temp memory space
                Counter++;
                if(SubDir->d_type == DT_DIR) //if the subdir is a directory and not a file
                {
                    char* path = new char[4096]; //create a temp val
                    strcpy(path,Directory);
                    strcat(path,"/");
                    strcat(path,SubDir->d_name); //undergo the same process as above in order to have a complete directory path
                    GetDirSize(path,Counter); //call recursively
                    free(path); //free the path memory space
                }
            }
            SubDir = readdir(Dir); //if not call readdir again and go deeper in the directory
        }
    }
    closedir(Dir); //close the directory
}