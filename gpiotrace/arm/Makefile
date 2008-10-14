CC = arm-palmos-gcc
CFLAGS = -Wall -O -s -I../include
LDFLAGS = -nostartfiles
OBJS = arm.o 

gpiotrace.arm: $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o gpiotrace.arm $(OBJS)

clean:
	rm -f $(OBJS) gpiotrace.arm
