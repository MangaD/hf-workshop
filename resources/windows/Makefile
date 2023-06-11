# Resources: HF Workshop
# Makefile created by MangaD

# Notes:
#  - If 'cmd /C' does not work, replace with 'cmd //C'

CMD = cmd /C

RES_FOLDER=.
RES     = $(wildcard $(RES_FOLDER)/*.rc)
RES_OBJ = $(RES:.rc=.o)

.PHONY: all clean

all: $(RES_OBJ)

$(RES_FOLDER)/%.o: $(RES_FOLDER)/%.rc $(RES_FOLDER)/resource.h
ifeq ($(OS),Windows_NT)
	windres $< $@
endif

clean:
ifeq ($(OS),Windows_NT)
	$(CMD) "del *.o *.aps $(RES_FOLDER)\*.o $(RES_FOLDER)\*.aps" 2>nul
else
	rm -f *.o *.aps $(RES_FOLDER)/*.o $(RES_FOLDER)/*.aps
endif