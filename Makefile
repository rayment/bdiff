#
# Makefile
#
# Author       : Finn Rayment <finn@rayment.fr>
# Date created : 23/11/2021
#

DEBUG:=0
VERSION:=1.0.0
BINARY:=bdiff

PREFIX:=/usr/local
MAN_PREFIX:=/usr/local/man

CC:=gcc
LD:=$(CC)

CCFLAGS:=--std=c89
CXXFLAGS:=-Wall -Wextra -Wpedantic -Werror --pedantic-errors \
          -O3 -DVERSION=\"${VERSION}\"
LDFLAGS:=

ifeq ($(DEBUG),1)
CXXFLAGS+=-g -DDEBUG=1
endif

CSOURCES:=$(wildcard src/*.c) $(wildcard src/*/*.c)
COBJECTS:=$(patsubst src/%,obj/%,$(patsubst %.c,%.o,$(CSOURCES)))
INCLUDE:=

DISTFILES:=Makefile $(BINARY).1
DISTDIRS:=src

all: $(COBJECTS)
	mkdir -p build
	$(LD) $(LDFLAGS) $(COBJECTS) $(LIBLIBS) -o build/$(BINARY)

obj/%.o: src/%.c
	mkdir -p $(shell dirname $@)
	$(CC) $(CCFLAGS) $(CXXFLAGS) $(LIBFLAGS) $(INCLUDE) -c $< -o $@

dist:
	rm -rf $(BINARY)-$(VERSION)
	mkdir -p $(BINARY)-$(VERSION)
	cp -R $(DISTFILES) $(DISTDIRS) $(BINARY)-$(VERSION)
	tar -cJf $(BINARY)-$(VERSION).tar.xz $(BINARY)-$(VERSION)
	rm -rf $(BINARY)-$(VERSION)

install:
	install -d $(PREFIX)/bin
	install -m 751 build/$(BINARY) $(PREFIX)/bin/$(BINARY)
	install -d $(MAN_PREFIX)/man1
	install -m 644 $(BINARY).1 $(MAN_PREFIX)/man1/$(BINARY).1

uninstall:
	rm -f $(PREFIX)/bin/$(BINARY)
	rm -f $(MAN_PREFIX)/man1/$(BINARY).1

clean:
	rm -rf build obj $(BINARY)

.PHONY: all dist install uninstall clean

