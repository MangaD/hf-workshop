# Build

## Get source

`git clone --recursive -j4 https://gitlab.com/MangaD/hf-workshop.git`

Init submodules:

`git submodule update --init --recursive`

Pull submodules:

`git submodule update --recursive --remote`

## Using GNU Make (Windows, Linux, Mac)

### Install build tools

#### Linux

**Debian/Ubuntu/Mint:** `$ sudo apt-get install build-essential g++ g++-multilib
libpthread-stubs0-dev`

For using zlib: `$ sudo apt-get install zlib1g-dev`  
If using XZ Utils: `$ sudo apt-get install liblzma-dev`  
For using minizip: `$ sudo apt-get install libminizip-dev`  
For using gettext: `$ sudo apt-get install gettext`  
For using readline (not in use): `$ sudo apt-get install libreadline-dev`  
For using editline: `$ sudo apt-get install libedit-dev`  

**RedHat/CentOS/Fedora:** `$ sudo yum install gcc gcc-c++ glibc-devel libgcc libstdc++-devel
glibc-devel.i686 libgcc.i686 libstdc++-devel.i686`

For using zlib: `$ sudo yum install zlib-devel zlib-devel.i686`  
If using XZ Utils: `$ sudo yum install xz-devel xz-devel.i686`  
For using minizip: `$ sudo yum install minizip-devel minizip-devel.i686`  
For using gettext: `$ sudo yum install gettext-devel gettext-devel.i686 intltool`  
For using readline (not in use): `$ sudo yum install readline-devel readline-devel.i686 `  
For using editline: `$ sudo yum install libedit-devel libedit-devel.i686`  

**Arch:** `$ sudo pacman -S base-devel`

For using zlib: `$ sudo pacman -S zlib`  
If using XZ Utils: `$ sudo pacman -S xz`  
For using minizip: `$ sudo pacman -S minizip`  
For using gettext: `$ sudo pacman -S gettext`  
For using readline (not in use): `$ sudo pacman -S readline`  
For using editline: `$ sudo pacman -S libedit`  

Compiler flags:

zlib: `-lz`  
xz: `-llzma`  
minizip: `-lminizip`  
readline  (not in use): `-lreadline` and on windows also `-ltermcap`  
editline: `-ledit` on Linux. `-ledit_static` on Windows.  
gettext: on windows `-lintl`  

#### Windows

##### MinGW Compiler

* Download latest [MinGW](http://sourceforge.net/projects/mingw-w64/files/Toolchains%20targetting%20Win32/Personal%20Builds/mingw-builds/installer/)

*   Install MinGW
    *   Architecture: i686 - for compiling 32 bit programs
    *   Architecture: x86_64 - for compiling 64 bit programs
    *   Threads: posix
    
* Copy "mingw32-make.exe" and rename the copy to "make.exe"

* Put the MinGW bin folder in the path

##### MinGW extra libraries

**32-bit:**

*   zlib: 
    <https://packages.msys2.org/package/mingw-w64-i686-zlib>
*   Gettext:
    <https://packages.msys2.org/package/mingw-w64-i686-gettext>
    <https://packages.msys2.org/package/mingw-w64-i686-expat>
    <https://packages.msys2.org/package/mingw-w64-i686-libiconv>
*   Editline:
    https://packages.msys2.org/package/mingw-w64-i686-wineditline
*   Readline (not in use):
    <https://packages.msys2.org/package/mingw-w64-i686-readline>
    <https://packages.msys2.org/package/mingw-w64-i686-termcap>
*   Minizip:
    https://packages.msys2.org/package/mingw-w64-i686-minizip

**64-bit:**

- zlib: 
  <https://packages.msys2.org/package/mingw-w64-x86_64-zlib>
- Gettext:
  <https://packages.msys2.org/package/mingw-w64-x86_64-gettext>
  <https://packages.msys2.org/package/mingw-w64-x86_64-expat>
  <https://packages.msys2.org/package/mingw-w64-x86_64-libiconv>
- Editline:
  https://packages.msys2.org/package/mingw-w64-x86_64-wineditline
- Readline (not in use):
  <https://packages.msys2.org/package/mingw-w64-x86_64-readline>
  <https://packages.msys2.org/package/mingw-w64-x86_64-termcap>
- Minizip:
  https://packages.msys2.org/package/mingw-w64-x86_64-minizip

If using XZ Utils:

*   Download XZ Utils for MinGW (only available for 32-bit): https://sourceforge.net/projects/mingw/files/MinGW/Extension/xz/xz-5.0.3-2/
    * File to download is `liblzma-5.0.3-2-mingw32-dev.tar.lzma`
	* Open file with 7-zip, open file inside with 7-zip
	* Copy `include` and `lib` to `C:\mingw-w64\mingw32\i686-w64-mingw32`

### Build

Use `$ make debug` for debug and `$ make release` for release. If compiling for a specific architecture use:

##### Debug 32-bit
`$ make debug32`

##### Debug 64-bit
`$ make debug64`

##### Release 32-bit
`$ make release32`

##### Release 64-bit
`$ make release64`

##### Release Windows XP
`$ make winxp`

Note: It is possible to compile with Clang by setting CXX environment variable to clang++. `$ CXX=clang++ make`

## Using cmake (Windows, Linux and Mac)

*CMake 2.8.12 or higher is required.*

### Install build tools

Debian/Ubuntu/Mint: `$ sudo apt-get install build-essential g++ g++-multilib libpthread-stubs0-dev zlib1g-dev liblzma-dev libminizip-dev gettext libedit-dev cmake`

RedHat/CentOS/Fedora: `$ sudo yum install gcc gcc-c++ glibc-devel libgcc libstdc++-devel
glibc-devel.i686 libgcc.i686 libstdc++-devel.i686 zlib-devel zlib-devel.i686
xz-devel xz-devel.i686 minizip-devel minizip-devel.i686 gettext-devel gettext-devel.i686 intltool libedit-devel libedit-devel.i686 cmake`

Arch: `$ sudo pacman -S base-devel zlib xz minizip gettext libedit cmake`

Windows: Download [Cmake](https://cmake.org/download/). You need either [MinGW](http://sourceforge.net/projects/mingw-w64/files/Toolchains%20targetting%20Win32/Personal%20Builds/mingw-builds/installer/) or [Visual Studio](https://www.visualstudio.com/).

### Build

```sh
$ mkdir build
$ cd build
```

Use `-DCMAKE_BUILD_TYPE:STRING=Debug` for debug and `-DCMAKE_BUILD_TYPE:STRING=Release` for release.

Generate Unix Makefile `$ cmake ..`

Generate Unix CodeBlocks project: `$ cmake -G "CodeBlocks - Unix Makefiles" ..`

Generate MinGW Makefile: `$ cmake -G "MinGW Makefiles" ..`

Generate MinGW CodeBlocks project: `$ cmake -G "CodeBlocks - MinGW Makefiles" ..`

Generate Visual Studio 2015 project: `$ cmake -G "Visual Studio 14 2015" ..`

Generate Visual Studio 2015 project (64-bit): `$ cmake -G "Visual Studio 14 2015 Win64" ..`

...

### Visual Studio

Install Windows 10 SDK: https://developer.microsoft.com/en-US/windows/downloads/windows-10-sdk

Using PowerShell:

```sh
cd .\hf-workshop\build

# Install vcpkg - A C++ package manager
# https://docs.microsoft.com/en-us/cpp/build/vcpkg?view=vs-2019
# https://devblogs.microsoft.com/cppblog/vcpkg-updates-static-linking-is-now-available/
git clone https://github.com/Microsoft/vcpkg
cd .\vcpkg
.\bootstrap-vcpkg.bat

# Search library example
.\vcpkg.exe search zlib

# Install libraries
.\vcpkg.exe install zlib:x86-windows-static
.\vcpkg.exe install zlib:x86-mingw-static
.\vcpkg.exe install zlib:x64-windows-static
.\vcpkg.exe install zlib:x64-mingw-static
.\vcpkg.exe install editline:x86-windows-static
.\vcpkg.exe install editline:x86-mingw-static
.\vcpkg.exe install editline:x64-windows-static
.\vcpkg.exe install editline:x64-mingw-static
.\vcpkg.exe install readline:x86-windows-static
.\vcpkg.exe install readline:x86-mingw-static
.\vcpkg.exe install readline:x64-windows-static
.\vcpkg.exe install readline:x64-mingw-static
.\vcpkg.exe install gettext:x86-windows-static
.\vcpkg.exe install gettext:x86-mingw-static
.\vcpkg.exe install gettext:x64-windows-static
.\vcpkg.exe install gettext:x64-mingw-static
.\vcpkg.exe install minizip:x86-windows-static
.\vcpkg.exe install minizip:x86-mingw-static
.\vcpkg.exe install minizip:x64-windows-static
.\vcpkg.exe install minizip:x64-mingw-static

# Make libraries available
.\vcpkg.exe integrate install

# Build project
cd ..
# Must be full path
cmake .. -DCMAKE_TOOLCHAIN_FILE=C:/Users/David/Desktop/hf-workshop/build/vcpkg/scripts/buildsystems/vcpkg.cmake -DVCPKG_TARGET_TRIPLET=x64-windows-static

# Open .sln with VS
# Change DEBUG to RELEASE
# HFWorkshop -> Properties -> Linker -> Input - Add:
#     vcpkg\installed\x64-windows-static\lib\readline.lib
#     vcpkg\installed\x64-windows-static\lib\libintl.lib
# build ALL_BUILD
```


## LZMA SDK

The files on `/libraries/lzma` were downloaded from [https://www.7-zip.org/sdk.html](https://www.7-zip.org/sdk.html).

Only the `C` folder matters. The Makefile is made by MangaD.
