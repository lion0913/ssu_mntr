ssu_mntr : main.o prompt.o ssu_mntr.o
	gcc main.o prompt.o -o ssu_mntr
	gcc ssu_mntr.o -o monitoring

main.o : main.c ssu_mntr.h
	gcc -c main.c

prmpt.o : prompt.c prompt.h
	gcc -c prompt.c

ssu_mntr.o : ssu_mntr.c ssu_mntr.h
	gcc -c ssu_mntr.c

clean : 
	rm *.o
	rm monitoring
	rm ssu_mntr
