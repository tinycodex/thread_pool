CFLAGS += -pthread -g -Wall -Werror
#LDFLAGS += -pthread


run: thread_pool

thread_pool:main.o thread_pool.o
	$(CC) -o $@ $^  $(CFLAGS) $(LDFLAGS)

clean:
	fd  -t x -x rm
	rm -f *.o thread_pool