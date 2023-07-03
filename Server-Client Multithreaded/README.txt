Lampropoulos Konstantinos

This implementation of a server client instance has 2 makefiles(one for the client and one for the server) and both are compiled using make.The server side of the program creates a local server
and the client side creates a client instance,which communicate through a TCP connection.The server runs indefinitely.The main purpose of this exercise is to create a server which handles multiple client requests
which are to locate a folder in the server disk and send it to the client.
An example execution of the client side looks like below:
./remoteclient -p 2020 -d folder1 -i 127.0.0.1
where,by the -p parameter we declare which port we want to connect,-d parameter specifies which folder client requests to copy and -i specifies the IP address of the server(since we are local any IP address will do).
An example execution of the serve side looks like below:
./dataserver -p 2020 -s 10 -q 8 -b 512
where,-p parameter specifies the port where the server will be initialized,-s specifies how many worker threads will be initialized(their functionality is explained below),
-q parameter specifies the size of the queue in which client requests are set to and -b specifies the block size (how many bytes are sent to the client each time).



Development Options:
If for any reason any of the makefiles doesn't work,server can be compiled by the command : g++ dataserver.cpp threads.cpp -lpthread -o dataserver,
and client can be compiled by the command : g++ -o remoteclient remoteclient.cpp.

There are comments in the code explaining every step.

Dataserver : 

1)WorkerThreads : They are initialized before the server is set to receive requests.They run indefinitely,since we suppose that server does that too.
They work with semaphores,by which they wait to receive an update that  there is an available file in the queue.They are never joined or detaches since server runs 24/7.
The whole process of sending the files of a folder and its contents was not implemented fully,and only all the files empty are being sent.There are comments of ideas to implement 
the functionality to send whole files by they are commented since they disrupted the execution.

2)Communication Threads : They are created for each client.They search the server disk to locate the folder(or single file) the client requests.If there is,they recursively send each file
to the queue,and if it is full they wait (by using semaphores) until there is a space in the queue open.They also inform the client of how many files where located in the directory 
so he doesn't cut the connection to the server earlier than expected and return corrupt files.Lastly,the await a report from the cleint that he received all the files.

3)Server: Server accepts connection requests from clients,informs client of the blocksize and creates the Communication Threads,

RemoteClient :
Creates the socket to connect with the server,receives tha packet size,and reads packet size bytes until he has received all the files
he requested.

The *.cpp files for each implementation are broken down to 2 folder,server and client.
