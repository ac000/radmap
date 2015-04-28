CC=gcc
CFLAGS=-Wall -Wno-switch -g -std=c99 -O2
LIBS=`pkg-config --libs clutter-1.0 glib-2.0 champlain-0.12`
INCS=`pkg-config --cflags clutter-1.0 glib-2.0 champlain-0.12` -lm

radmap: radmap.c vincenty_direct.c vincenty_direct.h
	$(CC) $(CFLAGS) -fPIC -c vincenty_direct.c
	$(CC) $(CFLAGS) radmap.c -o radmap vincenty_direct.o $(INCS) $(LIBS)

clean:
	rm -f radmap *.o
