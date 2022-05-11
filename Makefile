CFLAGS=-std=c11 -g -static
SRCS=$(wildcard *.c)  # container.c main.c parse.c codegen.c
OBJS=$(SRCS:.c=.o)  # container.o main.o parse.o codegen.o

1cc: $(OBJS)  # link file
	$(CC) -o 1cc $(OBJS) $(LDFLAGS) 

$(OBJS): 1cc.h

test: 1cc
	./test.sh

clean:
	rm -f 1cc *.o *~ tmp*

.PHONY: test clean