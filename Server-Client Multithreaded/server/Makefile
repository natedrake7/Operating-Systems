OBJS = dataserver.o threads.o
SOURCE = dataserver.cpp threads.cpp
HEADER = dataserver.hpp threads.hpp
OUT = dataserver
CC = g++
FLAGS = -c

$(OUT): $(OBJS)
		$(CC)  $(OBJS) -lpthread -o $@

dataserver.o: dataserver.cpp
			$(CC) $(FLAGS) dataserver.cpp

threads.o: threads.cpp
			$(CC) $(FLAGS) threads.cpp

clean: rm -f $(OBJS) $(OUT)