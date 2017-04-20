all: mxtool mxlibrary

mxtool: clean mxutil.o mxtool.o
	gcc mxutil.o mxtool.o -o mxtool -I/usr/include/libxml2 -lxml2 -fPIC

mxutil.o: mxutil.c mxutil.h
	gcc -g -c -Wall -std=c99 -I/usr/include/libxml2 mxutil.c -lxml2 -fPIC

mxtool.o: mxtool.c mxtool.h
	gcc -g -c -Wall -std=c99 -I/usr/include/libxml2 mxtool.c -lxml2 -fPIC


mxlibrary: 
	gcc -g -c -std=c99 mxlibrary.c -I/usr/include/libxml2 -fPIC `pkg-config --libs --cflags python3` -lxml2
	gcc -shared mxtool.o mxutil.o mxlibrary.o -o Mx.so -lxml2


clean:
	@ rm *.o -f
