Lampropoulos Konstantinos

This project is an implementation of InterProces-Communication(IPC).There is a main process(the parent process),
which is created by the execution of the program,and a number of child processes (the number of which are given as input from the command line).
An example of execution is below:
.\main -p PROCESS_VAL -t TEXT_FILE -r REQUEST_VAL -g GRAGING
where: PROCESS_VAL is the number of child processes and is specifed by the -p parameter,
TEXT_FILE the name of the text file we want to use(specified by the -t parameter),
REQUEST_VAL the number of requests each process will execute and is specifed by the -r parameter
and GRAGING which represents the grading of a file(how many lines compute a segment for the processes to read) and is 
specifed by the -g parameter.The parameters can be given at any combination.If the arguments are given in wrong order,like for example:
.\main -p tEXT_FILE -t PROCESS_VAL-r REQUEST_VAL -g GRAGING,
there will be undefined behaviour.The main purpose of this exercise is to create logfiles storing
the line of each segment that the processes requested to read,the times they read it(they might request to read it again)
and the time it took for them to be able to access it(waiting time).In order to do that a logfiles folder is created at execution,
and a temp file is created(it is automatically deleted at the end of execution).The temp file serves for the child processes to send the
IDs of the segments the want to read,and for the parent process to receive the IDs of the requested segments.The logfiles folder is recursively deleted
at the start of a new execution.There are more details of the project in the README.pdf file(but it is written in Greek) and in the comments of the *.c files.