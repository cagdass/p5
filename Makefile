

all:  libvsfs.a  app makedisk formatdisk

libvsfs.a: 	vdisk.c vsfs.c
	gcc -Wall -c vsfs.c
	ar -cvq  libvsfs.a vsfs.o
	ranlib libvsfs.a

app: 	app.c
	gcc -Wall -o app app.c  -L. -lvsfs

makedisk: makedisk.c
	gcc -Wall -o makedisk makedisk.c

formatdisk: formatdisk.c
	gcc -Wall -o formatdisk formatdisk.c -L. -lvsfs

clean: 
	rm -fr *.o *.a *~ a.out app makedisk formatdisk
