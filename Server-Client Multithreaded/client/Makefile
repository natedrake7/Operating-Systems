OBJS = remoteclient.o dirfuncs.o
SOURCE = remoteclient.cpp dirfuncs.cpp
HEADER = remoteclient.hpp dirfuncs.hpp
OUT = remoteclient
CC = g++
FLAGS = -c

$(OUT): $(OBJS)
		$(CC)  $(OBJS) -o $@

remoteclient.o: remoteclient.cpp
			$(CC) $(FLAGS) remoteclient.cpp

dirfuncs.o: dirfuncs.cpp
			$(CC) $(FLAGS) dirfuncs.cpp

clean: rm -f $(OBJS) $(OUT)