CC=gcc
CFLAGS=-Wall -Wno-switch -g -std=c99 -O2
LIBS=`pkg-config --libs clutter-1.0 glib-2.0 champlain-0.12` -lm -lac
INCS=`pkg-config --cflags clutter-1.0 glib-2.0 champlain-0.12`

radmap: radmap.c
	$(CC) $(CFLAGS) radmap.c -o radmap $(INCS) $(LIBS)

clean:
	rm -f radmap *.o
