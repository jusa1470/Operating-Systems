#Makefile
HEADER = multi_lookup.h util.h
CFLAGS = -Wall -Wextra -c
FFLAGS = -pthread
CC = gcc


default: multi_lookup

multi_lookup.o: multi_lookup.c $(HEADER) util.c
	$(CC) $(CFLAGS) multi_lookup.c -o multi_lookup.o

multi_lookup: multi_lookup.o
	$(CC) $(FFLAGS) multi_lookup.o -o multi_lookup

clean:
	-rm -f multi_lookup
	-rm -f multi_lookup.o
	-rm -f results.txt
	-rm -f serviced.txt
