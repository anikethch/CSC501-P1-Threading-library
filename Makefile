mythread.a: mythread.o mythreadlib.o
	ar rcs mythread.a mythread.o mythreadlib.o
mythread.o: mythread.c mythread.h mythreadlib.c mythreadlib.h
	gcc -c mythread.c  mythreadlib.c
mythreadlib.o: mythreadlib.c mythreadlib.h 
	gcc -c mythreadlib.c -o mythreadlib.o
