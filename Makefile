#
# Makefile
#
# Author       : Finn Rayment <finn@rayment.fr>
# Date created : 23/11/2021
#

DEBUG:=0
VERSION:=1.0.0
BINARY:=bdiff

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

DISTFILES:=Makefile
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

clean:
	rm -rf build obj $(BINARY)

.PHONY: all dist clean

