#include "remoteclient.hpp"
#include "dirfuncs.hpp"

int main(int argc,char** argv)
{
    if(argc != 7)
    {
        if(argc > 7)
            cout<<"Too many arguments were given!"<<endl;
        else
            cout<<"The Client needs more arguments to be initialized correctly!"<<endl;
        exit(ARGS_ERROR);
    }
    char* ServerIP = new char[50];
    char* Directory = new char[4096];
    int port;
    GetArgs(argc,argv,&ServerIP,&port,&Directory);
    ConnectToServer(ServerIP,port,Directory);
    free(ServerIP); //free the dynamically allocated strings above
    free(Directory);
    return 0;
}

void GetArgs(int argc,char** argv,char** ServerIP,int* port,char** Directory)
{
    bool IP = false,portnum = false,Dirbool = false; //booleans to check if we have found the corresponding values
    for(int i = 0;i < argc;i++) //iterate argc times the argv array
    {
        if(IP == true) //if IP is found
        {
            strcpy(*ServerIP,argv[i]); //copy the string to the server ip val
            IP = false; //false so the if condition isn't satisfied at the next iteration 
        }
        else if(portnum == true) //if the portnumber is found
        {
            *port = atoi(argv[i]); //convert the string given in the input to and integer
            portnum = false; //false so the if condition isn't satisfied at the next iteration 
        }
        else if(Dirbool == true)
        {
            strcpy(*Directory,argv[i]); //copy the string to the directory val
            Dirbool = false; //false so the if condition isn't satisfied at the next iteration 
        }
        else if(strcmp(argv[i],"-i") == 0) //check if the argv param is -i
            IP = true; //notify for next iteration
        else if(strcmp(argv[i],"-p") == 0) //check if the argv param is -p
            portnum = true; //notify for next iteration
        else if(strcmp(argv[i],"-d") == 0) //check if argv param is -d
            Dirbool = true; //notify for next iteration
    }
}

void ConnectToServer(char* ServerIP,int port,char* Directory)
{
    int sockfd,connfd;
    struct sockaddr_in server,client;
    sockfd = socket(AF_INET,SOCK_STREAM,0); //initialze a TCP socket
    if(sockfd == -1) //check for errors
    {
        perror("Failed to create socket"); //error message
        exit(SOCKET_ERROR); //exit error status
    }
    server.sin_family  = AF_INET;
    server.sin_addr.s_addr = inet_addr(ServerIP); //pass the IP
    server.sin_port = htons(port); //pass the port
    connfd = connect(sockfd,(struct sockaddr *)&server,sizeof(server)); //connect to the server
    if(connfd == -1) //if connect fails
    {
        perror("Failed to connect to server"); //error message
        exit(CONNECTION_ERROR); //exit error status
    }
    else
        cout<<"Connected to server with IP : "<<ServerIP<<endl; //print connection successful message
    int bytes = write(sockfd,Directory,strlen(Directory) + 1); //write the directory to be copied to the server
    if(bytes  == -1) //check if write was successful
    {
        perror("Failed to send Directory path to server"); //print error message
        exit(CONNECTION_ERROR); //exit Error status
    }
    int bytesread,BlockSize;
    bytesread = read(sockfd,(char*)&BlockSize,sizeof(int)); //read the blocksize from the server
    if(bytesread == -1) //if read fails
    {
        perror("Error receiving Block Size from server");     //error message
        exit(READ_ERROR); //exit error status
    }
    char* File = new char[BlockSize];
    char* Dir = new char[BlockSize];
    //struct stat st;
    int counter = 1,FileCount = 0;
    while(1)
    {
        bytesread = read(sockfd,File,BlockSize); //get the file directory
        if(bytesread == -1) //if read fails
        {
            perror("Error reading files from server");
            exit(READ_ERROR);
        }
        if(atoi(File) == 0) //if atoi is 0 then the string is not a number
        {
            strcpy(Dir,File); //hold the file name to a temp variable
            CreateDirectory(File,sockfd); //create the directory
        }
        else
            FileCount = atoi(File); //if atoi is not 0 ,this is the file count
        GetDirSize(Directory,counter); //check recursively each time if the directory size is equal to the file count
        if(FileCount == counter)
        {
            char* temp = new char[100];
            strcpy(temp,"Done"); //then
            bytes = write(sockfd,temp,strlen(temp) + 1); //send a termination messsage to the communication thread
            if(bytes == -1)
            {
                perror("Failed to send termination message to server");
                exit(CONNECTION_ERROR);
            }
            free(temp);
            break; //break the loop
        }
       /* if(stat(Dir,&st) == 0 && !S_ISDIR(st.st_mode))
        {
            fd = open(Dir,O_WRONLY|O_APPEND,0644);
            char* message = new char[BlockSize];
            int bytes;
            if(fd == -1)
            {
                perror("Failed to open file");
                exit(READ_ERROR);
            }
            while(1)
            {
                bytesread = read(sockfd,message,BlockSize);
                if(bytesread == -1)
                {
                    perror("Failed to read file contents");
                    exit(READ_ERROR);
                }
                received+=bytesread;
                if(received == Size)
                    break;
                bytes = write(fd,message,bytesread);
                if(bytes == -1)
                {
                    perror("Failed to write file contents to file");
                    exit(READ_ERROR);
                }
                memset(message,0,BlockSize);
            }
        }*/
        memset(File,0,BlockSize); //allocate new space
        memset(Dir,0,BlockSize);
        counter = 1;
    }
    free(File); //free the space used
    free(Dir);
    cout<<"Ending connection with server!"<<endl;
}