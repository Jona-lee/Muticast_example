CC = arm-none-linux-gnueabi-gcc
STRIP = arm-none-linux-gnueabi-strip

all:mcast mcastx86

mcast:mcast.o
    $(CC) -o $@ $^

mcastx86.o:mcast.c
    gcc -c -o $@ $^ 
mcastx86:mcastx86.o
    gcc -o $@ $^

.PHONY:clean install
clean:
    ${RM} mcast mcastx86 *.o

install:
    $(STRIP) mcast
