# ![logo](resources/logo.png)

[![pipeline status](https://gitlab.com/MangaD/hf-workshop/badges/master/pipeline.svg?style=flat-square)](https://gitlab.com/MangaD/hf-workshop/commits/master) [![coverage report](https://gitlab.com/MangaD/hf-workshop/badges/master/coverage.svg?style=flat-square)](https://gitlab.com/MangaD/hf-workshop/commits/master) [![license](https://img.shields.io/badge/license-MIT-red?style=flat-square)](LICENSE)

Hero Fighter Workshop is a program for replacing stages, sounds, images and some data in [Hero Fighter](http://herofighter.com/index_en.html). The game's stages are written in XML format, which HF can interpret. Data files are exported in JSON format, converted from AMF format, and some also include PNG files.

## Download & User Guide

- **[HF Workshop](https://hf-empire.com/forum/showthread.php?tid=317)**
- **[HF & Tools](https://www.mediafire.com/folder/mue8oxr6muaa6/hf_disassembled_resources)**

## Build

See [documentation/BUILD.md](documentation/BUILD.md).

## Methodology

See [documentation/METHODOLOGY.md](documentation/METHODOLOGY.md).

## Contributing

See [documentation/CONTRIBUTING.md](documentation/CONTRIBUTING.md).

## Documentation

Documentation is generated using Doxygen (and doxywizard). Needs code review.

## Copyright

Copyright (c) 2019-2020 David Gonçalves

## License

This project uses the MIT [license](LICENSE).

## Third-Party Libraries

| **Project**                                                  | **Author(s)**                                                | **License**                                                  | **Comments**                                                 |
| ------------------------------------------------------------ | ------------------------------------------------------------ | ------------------------------------------------------------ | ------------------------------------------------------------ |
| [rlutil](https://github.com/tapio/rlutil)                    | [Tapio Vierros](https://github.com/tapio)                    | [WTFPL-2.0](https://github.com/tapio/rlutil/blob/master/docs/License.txt) | Used for cross-platform console coloured text.               |
| [zlib](https://www.zlib.net/)                                | [Jean-loup Gailly](http://gailly.net/) (compression) and [Mark Adler](http://en.wikipedia.org/wiki/Mark_Adler) (decompression) | [zlib](https://zlib.net/zlib_license.html)                   | Used for compressing and decompressing AMF data.             |
| [minizip](https://www.winimage.com/zLibDll/minizip.html)     | [Gilles Vollant](https://www.winimage.com/zLibDll/minizip.html) | [zlib](http://www.nf.mpg.de/vinci3/doc/license-minizip.html) | Used for exporting / importing character data files in a zip file. It uses the [zlib library](https://www.zlib.net/). |
| [GNU Readline](https://git.savannah.gnu.org/cgit/readline.git) | Original by [Brian Fox](https://en.wikipedia.org/wiki/Brian_Fox_(computer_programmer)) and maintained by [Chet Ramey](https://tiswww.case.edu/php/chet/) | [GPL-3](http://www.gnu.org/licenses/gpl.html)                | **Not in use.** Alternative for editline. Used for console input auto-complete, input history, input shortcuts. Uses the [termcap library](https://ftp.gnu.org/gnu/termcap/) ([GPL-2.0](https://ftp.gnu.org/gnu/termcap/)) on Windows. |
| [Editline](https://thrysoee.dk/editline/)                    | [Jess Thrysoee](https://thrysoee.dk/)                        | [BSD 3-Clause](https://thrysoee.dk/editline/)                | Used for console input auto-complete, input history, input shortcuts. Alternative to GNU Readline. Depends on [libncurses](https://invisible-island.net/ncurses/ncurses.html) [X11 License](https://invisible-island.net/ncurses/ncurses-license.html). |
| [WinEditLine](http://mingweditline.sourceforge.net/)         | [Paolo Tosco](http://mingweditline.sourceforge.net/)         | [BSD 3-Clause](http://mingweditline.sourceforge.net/?License) | Editline equivalent for MinGW.                               |
| [GNU gettext libintl](https://www.gnu.org/software/gettext/) | Original by [Sun Microsystems](https://en.wikipedia.org/wiki/Sun_Microsystems), various developers. | [LGPL-2.1](https://www.gnu.org/software/gettext/manual/html_node/Licenses.html) | Used for internationalization and localization. On Windows uses [iconv library](https://www.gnu.org/software/libiconv/) ([LGPL](https://www.gnu.org/software/libiconv/)) and [Expat library](https://libexpat.github.io/) ([MIT](https://github.com/libexpat/libexpat/blob/master/expat/COPYING)). |
| [libswf](https://gitlab.com/MangaD/libswf)                   | [David Gonçalves](https://gitlab.com/MangaD)                 | [MIT License](https://gitlab.com/MangaD/libswf/-/blob/master/LICENSE) | Used for manipulating the SWF file. Makes use of 3rd-party libraries. Check its page for information on them and the respective licenses. |

Table generated with: https://www.tablesgenerator.com/markdown_tables

A copy of the licenses can be found in [documentation/additional_license_information.txt](documentation/additional_license_information.txt) and must be distributed alongside the binary.

## Credits

[Hero Fighter](http://herofighter.com) is property of Marti Wong. HF Workshop was developed by MangaD as a tribute to the game and to keep it alive with many customizations possible in-game.

Project badge images created with https://shields.io/.
