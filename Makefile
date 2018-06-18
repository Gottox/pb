demo: demo.c pb.c pb.h
	$(CC) -g -pthreads demo.c pb.c -o $@ $(CFLAGS) $(LDFLAGS)

.PHONY: clean

clean:
	rm demo
