all: conv
clean:
	rm -f conv *.o

.PHONY: all clean install

ifeq ($(PREFIX),)
    PREFIX := /usr/local
endif

CXX = gcc
CFLAGS = -c -Wall -Wextra

conv: conv.o
	$(CXX) -lm -o $@ $^

conv.o: conv.c
	$(CXX) $(CFLAGS) conv.c

install: conv
	install ./conv $(DESTDIR)$(PREFIX)/bin
