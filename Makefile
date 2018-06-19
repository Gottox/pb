CFLAGS = -Wpedantic -Wall -Werror -g

demo: demo.c pb.c pb.h
	$(CC) -pthreads demo.c pb.c -o $@ $(CFLAGS) $(LDFLAGS)

.PHONY: clean

clean:
	rm demo
