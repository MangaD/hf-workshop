# Project: HF Workshop
# Makefile created by MangaD

# Notes:
#  - If 'cmd /C' does not work, replace with 'cmd //C'

CMD = cmd /C

# Necessary for echo -e
SHELL = bash

APP_NAME    = HFWorkshop
CXX        ?= g++
SRC_FOLDER  = source
OBJ_FOLDER  = build
RES_FOLDER  = resources
LANG_FOLDER = resources/languages
LIB_FOLDER  = libraries
INCLUDES    = -isystem include
# libswf include
INCLUDES   += -isystem $(LIB_FOLDER)/libswf/libswf/source
INCLUDES   += -isystem $(LIB_FOLDER)/libswf/libswf/include
# rlutil include
INCLUDES   += -isystem $(LIB_FOLDER)
BIN_FOLDER  = bin

# By default, we build for release
BUILD=release

SUBDIRS = $(LIB_FOLDER) $(LANG_FOLDER)
ifeq ($(OS),Windows_NT)
RES_FOLDER := $(RES_FOLDER)/windows
SUBDIRS += $(RES_FOLDER)
else
RES_FOLDER := $(RES_FOLDER)/linux
endif

# SRC is a list of the cpp files in the source directory.
SRC = $(wildcard $(SRC_FOLDER)/*.cpp)
# OBJ is a list of the object files to be generated in the objects directory.
OBJ = $(subst $(SRC_FOLDER),$(OBJ_FOLDER), $(SRC:.cpp=.o))
RES = $(wildcard $(RES_FOLDER)/*.rc)

RES_OBJ = $(RES:.rc=.o)

ifeq ($(OS),Windows_NT)
OBJ += $(RES_OBJ)
endif

DEFINES=

# Windows should work with Unicode
ifeq ($(OS),Windows_NT)
DEFINES += -DUNICODE -D_UNICODE
endif

ifneq ($(BUILD),release)
WARNINGS = -Wall -Wextra -pedantic -Wmain -Weffc++ -Wswitch-default \
	-Wswitch-enum -Wmissing-include-dirs -Wmissing-declarations -Wunreachable-code -Winline \
	-Wfloat-equal -Wundef -Wcast-align -Wredundant-decls -Winit-self -Wshadow -Wnon-virtual-dtor \
	-Wconversion #-Wno-useless-cast
ifeq ($(CXX),g++)
WARNINGS += -Wzero-as-null-pointer-constant #-Wuseless-cast
else
ifeq ($(CXX),clang++)
#http://clang.llvm.org/docs/ThreadSafetyAnalysis.html
WARNINGS += -Wthread-safety
endif
endif
endif

# Debug or Release #
ifeq ($(BUILD),release)
# "Release" build - optimization, no debug symbols and no assertions (if they exist)
OPTIMIZE    = -O3 -s -DNDEBUG
ifeq ($(OS),Windows_NT)
STATIC      = -static -static-libgcc -static-libstdc++
BIN_FOLDER := $(BIN_FOLDER)\release
else
BIN_FOLDER := $(BIN_FOLDER)/release
endif
else
# "Debug" build - no optimization, with debugging symbols and assertions (if they exist), and sanitize
OPTIMIZE    = -O0 -g -DDEBUG
ifneq ($(OS),Windows_NT)
# Coverage: --coverage -fprofile-arcs -ftest-coverage
OPTIMIZE    := -fsanitize=undefined,address -fno-sanitize-recover=all
ifeq ($(CXX),g++)
OPTIMIZE    := --coverage -fprofile-arcs -ftest-coverage
endif # g++
BIN_FOLDER := $(BIN_FOLDER)/debug
else
BIN_FOLDER := $(BIN_FOLDER)\debug
endif # win
DEFINES    += -DSWF_DEBUG_BUILD -DZIP_DEBUG_BUILD
endif # release

# ARCHITECTURE (x64 or x86) #
# Do NOT use spaces in the filename nor special characters
ifeq ($(ARCH),x64)
BIN          = $(BIN_FOLDER)/$(APP_NAME)_x64
ARCHITECTURE = -m64
else ifeq ($(ARCH),x86)
BIN          = $(BIN_FOLDER)/$(APP_NAME)_x86
ARCHITECTURE = -m32
else
BIN          = $(BIN_FOLDER)/$(APP_NAME)
endif

ifeq ($(OS),Windows_NT)
BIN := $(BIN).exe
endif

# LIBRARIES #
LDFLAGS = -L$(LIB_FOLDER)
LDFLAGS+= -L$(LIB_FOLDER)/libswf/libswf/libraries
# libswf: -lswf
#     zlib: -lz
#     lzma sdk: -llzmasdk
#     xz: -llzma
#     lodepng: -llodepng
#   Linking of lzma sdk and lodepng must come after
# gnu readline: -lreadline
# minicip: -lminizip
LDLIBS += -lminizip -lswf -lz -llzmasdk -llodepng # -llzma

# CAREFUL: GNU Readline uses the GPLv3 license! This has implications in
#          the rights you have over the code you link it against.
#LDLIBS += -lreadline

# editline, or libedit, BSD 3-clause
# depends on libncurses
ifeq ($(OS),Windows_NT)
LDLIBS += -ledit_static
else
LDLIBS += -ledit
#-lncurses
endif

ifeq ($(OS),Windows_NT)
# -ltermcap: required by GNU Readline, GPL-2.0
#LDLIBS += -ltermcap

# -lintl: gettext on windows, LGPL-2.1
LDLIBS += -lintl
endif
LDLIBS += $(STATIC)
LDLIBS += $(ARCHITECTURE)

# Add to LDLIBS `-fsanitize=undefined -fsanitize=thread` OR `-fsanitize=memory -fsanitize-memory-track-origins=2` OR `-fsanitize=address`
# when compiling with Clang for testing. Use `-fno-omit-frame-pointer` with any of them. If there are errors at runtime, they will be printed to stderr.
# Note: In Ubuntu I needed to `sudo ln -s /usr/bin/llvm-symbolizer-3.8 /usr/bin/llvm-symbolizer`
# for `-fsanitize=memory -fsanitize-memory-track-origins=2`. But still got false positives, probably needed to:
# http://stackoverflow.com/questions/20617788/using-memory-sanitizer-with-libstdc
#
# -lgcov - coverage
ifneq ($(BUILD),release)
ifneq ($(OS),Windows_NT)
LDLIBS += -fsanitize=undefined,address -fno-sanitize-recover=all
endif # win
ifeq ($(CXX),g++)
LDLIBS += -lgcov
endif # converage
endif # release

CXXFLAGS = $(INCLUDES) $(ARCHITECTURE) -std=c++17 $(DEFINES) $(WARNINGS) $(OPTIMIZE)

all: $(BIN)
.PHONY : all clean run run32 run64 debug release release32 release64 debug32 debug64 winxp $(SUBDIRS) install uninstall pack

$(BIN): $(OBJ_FOLDER) $(BIN_FOLDER) $(SUBDIRS) $(OBJ)
	$(CXX) $(OBJ) -o $@ $(LDFLAGS) $(LDLIBS)


$(OBJ_FOLDER)/%.o: $(SRC_FOLDER)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@


# DEPENDENCIES #
$(OBJ_FOLDER)/main.o: $(SRC_FOLDER)/hf_workshop.hpp
$(OBJ_FOLDER)/utils.o: $(SRC_FOLDER)/utils.hpp
$(OBJ_FOLDER)/hf_workshop.o: $(SRC_FOLDER)/hf_workshop.hpp $(SRC_FOLDER)/utils.hpp \
					$(SRC_FOLDER)/io_wrapper.hpp $(SRC_FOLDER)/minizip_wrapper.hpp
$(OBJ_FOLDER)/io_wrapper.o: $(SRC_FOLDER)/io_wrapper.hpp
$(OBJ_FOLDER)/minizip_wrapper.o: $(SRC_FOLDER)/minizip_wrapper.hpp


$(SUBDIRS):
	$(MAKE) -C $@ BUILD=$(BUILD) ARCH=$(ARCH)

debug:
	$(MAKE) BUILD=debug
release:
	$(MAKE) BUILD=release
release32:
	$(MAKE) BUILD=release ARCH=x86
release64:
	$(MAKE) BUILD=release ARCH=x64
debug32:
	$(MAKE) BUILD=debug ARCH=x86
debug64:
	$(MAKE) BUILD=debug ARCH=x64
winxp:
	$(MAKE) BUILD=release ARCH=x86 DEFINES=-DWINVER=0x0501

ifneq ($(OS),Windows_NT)
install: bin/release/$(shell basename $(BIN))
	mkdir -p /usr/local/bin
	mkdir -p /usr/local/share/applications/
	mkdir -p /usr/local/share/icons/hicolor/32x32/apps
	cp $< /usr/local/bin/$(shell basename $(BIN))
	cp $(RES_FOLDER)/$(APP_NAME).desktop /usr/local/share/applications/
	cp $(RES_FOLDER)/icon.png /usr/local/share/icons/hicolor/32x32/apps/$(APP_NAME).png
endif

uninstall:
ifneq ($(OS),Windows_NT)
	rm -f /usr/local/bin/$(shell basename $(BIN))
	rm -f /usr/local/share/applications/$(APP_NAME).desktop
	rm -f /usr/local/share/icons/hicolor/32x32/apps/$(APP_NAME).png
endif

clean:
ifeq ($(OS),Windows_NT)
	FOR %%D IN ($(SUBDIRS)) DO $(MAKE) -C %%D clean
else
	@$(foreach dir,$(SUBDIRS),$(MAKE) -C $(dir) clean;)
endif
ifeq ($(OS),Windows_NT)
	$(CMD) "del *.o $(SRC_FOLDER)\*.o $(OBJ_FOLDER)\*.o $(OBJ_FOLDER)\*.gcda $(OBJ_FOLDER)\*.gcno" 2>nul
else
	rm -f *.o $(SRC_FOLDER)/*.o $(OBJ_FOLDER)/*.o $(OBJ_FOLDER)/*.gcda $(OBJ_FOLDER)/*.gcno
endif

pack:
ifeq ($(OS),Windows_NT)
# PowerShell
	COPY /Y resources\windows\SA.exe $(BIN_FOLDER)\SA.exe
	powershell.exe Compress-Archive -Force -LiteralPath $(BIN_FOLDER)\$(APP_NAME).exe, $(BIN_FOLDER)\SA.exe, resources\hf.swf -DestinationPath $(BIN_FOLDER)\$(APP_NAME)
	del $(BIN_FOLDER)\SA.exe
else
	cp resources/windows/SA.exe $(BIN_FOLDER)/SA.exe
	zip -FSrjq $(BIN_FOLDER)/$(APP_NAME).zip $(BIN_FOLDER)/$(APP_NAME).exe
	cd $(BIN_FOLDER) && zip -rq $(APP_NAME).zip SA.exe && rm SA.exe
	cd resources && zip -rq ../$(BIN_FOLDER)/$(APP_NAME).zip hf.swf

endif

run:
	$(BIN)

run32:
	$(MAKE) "ARCH=x86" run
run64:
	$(MAKE) "ARCH=x64" run

$(OBJ_FOLDER):
ifeq ($(OS),Windows_NT)
	$(CMD) "mkdir $(OBJ_FOLDER)"
else
	mkdir -p $(OBJ_FOLDER)
endif

$(BIN_FOLDER):
ifeq ($(OS),Windows_NT)
	$(CMD) "mkdir $(BIN_FOLDER)"
else
	mkdir -p $(BIN_FOLDER)
endif
