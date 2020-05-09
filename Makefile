ssu_mntr : main.o monitoring.o prompt.o
	gcc main.o prompt.o -o ssu_mntr
	gcc monitoring.o -o monitoring

main.o : main.c monitoring.h
	gcc -c main.c

prompt.o : prompt.c monitoring.c monitoring.h
	gcc -c prompt.c
	gcc -c monitoring.c

monitoring.o : monitoring.c monitoring.h
	gcc -c monitoring.c

clean : 
	rm *.o
	rm monitoring
	rm ssu_mntr
