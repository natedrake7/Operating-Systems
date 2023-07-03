
OBJS = Parent.o Child.o Directory.o
SOURCE = Parent.c Child.c Directory.c
HEADER = Parent.h Child.h Directory.h
OUT = main
CC = gcc
FLAGS = -c

$(OUT): $(OBJS)
		$(CC) $(OBJS) -o $@

Parent.o: Parent.c
		  $(CC) $(FLAGS) Parent.c

Child.o: Child.c
		 $(CC) $(FLAGS) Child.c

Directory.o: Directory.c
			 $(CC) $(FLAGS) Directory.c

clean:
		rm -f ($OBJS) $(OUT)

count:
	wc $(SOURCE) $(HEADER)
