# Makefile created by MangaD

CMD=cmd /C

DIRS = libswf

.PHONY: all clean

all: libswf.a

libswf.a:
	$(MAKE) -C libswf BUILD=$(BUILD) ARCH=$(ARCH)

clean:
ifeq ($(OS),Windows_NT)
	FOR %%D IN ($(DIRS)) DO $(MAKE) -C %%D clean
	$(CMD) "del *.a" 2>nul
else
	for d in $(DIRS); do $(MAKE) -C $$d clean; done
	rm -f *.a
endif
