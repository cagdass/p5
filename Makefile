

all: libvsfs.a  app makedisk

libvsfs.a: 	vsfs.c
	gcc -Wall -c vsfs.c
	ar -cvq libvsfs.a vsfs.o
	ranlib libvsfs.a

app: 	app.c
	gcc -Wall -o app app.c  -L. -lvsfs


makedisk: makedisk.c
	gcc -Wall -o makedisk makedisk.c

clean: 
	rm -fr *.o *.a *~ a.out app makedisk
