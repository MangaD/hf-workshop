# Library: libswf
# https://gitlab.com/MangaD/libswf

# Makefile created by MangaD

CMD = cmd /C

ifeq ($(OS),Windows_NT)
PROG = ..\libswf.a
else
PROG = ../libswf.a
endif
CC = gcc
CXX ?= g++
AR      = ar
ARFLAGS = rcvs
SRC_FOLDER = libswf
OBJ_FOLDER = .
LIB =
RM = rm -f

BIN_FOLDER =
ifeq ($(BUILD),release)
ifeq ($(OS),Windows_NT)
BIN_FOLDER := $(SRC_FOLDER)\bin\release
else
BIN_FOLDER := $(SRC_FOLDER)/bin/release
endif
else
ifneq ($(OS),Windows_NT)
BIN_FOLDER := $(SRC_FOLDER)/bin/debug
else
BIN_FOLDER := $(SRC_FOLDER)\bin\debug
endif # win
endif # release

.PHONY: all clean

all: $(PROG)

$(PROG):
	$(MAKE) -C $(SRC_FOLDER) BUILD=$(BUILD) ARCH=$(ARCH)
ifeq ($(OS),Windows_NT)
	$(CMD) "copy $(BIN_FOLDER)\*.a .." 2>nul
else
	cp $(BIN_FOLDER)/*.a ..
endif

clean:
	$(MAKE) -C $(SRC_FOLDER) clean
ifeq ($(OS),Windows_NT)
	$(CMD) "del $(PROG)" 2>nul
else
	rm -f $(PROG)
endif
