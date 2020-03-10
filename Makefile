.PHONY: all clean 

CPLUSPLUS?=g++

CPPFLAGS=-std=c++17 -W -Wall -O3
LDFLAGS=

SRC=$(wildcard *.cc)
BIN=$(SRC:%.cc=bin/%)
LOCAL_HEADERS_DEPS=$(shell find . -maxdepth 1 -name '*.h' -exec echo '{}' ';' | sort -g)

PERMSIGN= /
OS=$(shell uname)
ifeq ($(OS),Darwin)
  CPPFLAGS+= -stdlib=libc++ -x objective-c++ -fobjc-arc
  # Reset `LDFLAGS` with the new Darwin-specific value.
  LDFLAGS= -framework Foundation
  PERMSIGN= +
endif

default: all

all: ${BIN}
	./bin/iq-xoxo

clean:
	rm -rf bin core

bin/%: %.cc ${LOCAL_HEADERS_DEPS}
	@(mkdir -p bin)
	${CPLUSPLUS} ${CPPFLAGS} -o "$@" "$<" ${LDFLAGS}
