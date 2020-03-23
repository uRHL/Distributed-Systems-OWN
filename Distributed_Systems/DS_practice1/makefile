
CFLAGS = -g -Wall
CC = gcc
	
all: server.c client.c array.c 
	$(CC) $(CFLAGS) -c array.c
	ar -rv libArray.a array.o
	$(CC) $(CFLAGS) -o server server.c -lrt -lpthread
	$(CC) $(CFLAGS) -o client client.c -lrt libArray.a

.PHONY : clean
clean:
	rm -f *.o
	rm -f $(LIB)
