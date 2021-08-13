CFLAGS = -Wall -Werror -O2
LDFLAGS = 

all: nanocom

clean:
	rm -fv *~ *.o

install: all
	install -m 0755 -o root -g root nanocom $(DESTDIR)/usr/local/bin/

distclean: clean
	rm -fv nanocom

nanocom: serial_raw.o nanocom.o screen.o
