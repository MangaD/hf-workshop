# Methodology


## Extracting the SWF from the original HF v0.7

There are two distinct release files of Hero Fighter. An SWF file which is played on the user's browser, called `Console_v0.7.0_secure.swf`, and an EXE file which is installed on the user's PC. The SWF file is encryped with MochiCrypt, if we attempt to use an SWF decompiler on it we will be seeing multiple SWF files, and the game is encrypted inside one of them using MochiCrypt. The EXE file was not compiled using the Adobe Flash Player projector, I don't know how it was compiled and the SWF file is encrypted as well (can't see it with hex editor).

In order to extract the SWF file of the game we use [SWF Memory Dumper](https://sites.google.com/site/forceprojectx/services/apps/memory_dumper), which is a program that extracts the SWF file from the memory while the game is running. The principle is that flash games need to be uncompressed and decrypted in memory, before flash can play them. So if we manage to get the game directly from the memory, we bypass zlib compression, mochicrypt encryption and stuff like that, and get a ready-to-hack file.

## Extracting the SWF from HFX

The SWF is inside the APK file, which is a ZIP file, in the path `assets/HeroFighterX.swf`. Unlike HF v0.7, this one is not encrypted.


## Editing the SWF

We use the [JPEXS Free Flash Decompiler](https://www.free-decompiler.com/flash/) program in order to inspect the HF SWF file. This program is useful to guide us on the SWF file structure and content so that we can mimic what it does. We could use this program for our needs entirely, but the idea is to keep the SWF a black box to the end user as to avoid too much hacking of the game (potentially harming the author's income), and also to automatize the process of replacing certain files.

More info on the SWF file structure can be found on the file `swf-file-format-spec.pdf` inside the `documentation` folder. Chapter 2 is particularly important. The [SWF Reference by Alexis](https://www.m2osw.com/swf_alexref) - part of the free SSWF project - is also very useful, it is inside the `documentation` folder with the name `SWF File Format Reference.pdf`

Other useful tools are an hex editor and a program to find differences between binary files (eg. [VBinDiff](https://www.cjmweb.net/vbindiff/)) so that we can see what modifications JPEXS does to the swf.

## Editing images

`swf-file-format-spec.pdf` - Page 139


## Editing sounds

`swf-file-format-spec.pdf` - Page 179


## Editing characters, backgrounds, data...

Character data is stored in binary files which consist of ByteArray's containing serialized objects. In Flash, objects are serialized in [Action Message Format](<https://en.wikipedia.org/wiki/Action_Message_Format>). There are `Lmi`, `Spt`, `Bg` and `gdat` files. There is one `Lmi` and one `Spt` file for each character. There is one `Bg` file and one `gdat` file.

All these files contain zlib compressed data in one AMF3 ByteArray. The decompressed data starts with an UTF string indicating their type, with can be `limbInfo`, `Spt`, `Bg` and `gdat`. The UTF string is prefixed with an unsigned short indicating its length in bytes as indicated [here](https://help.adobe.com/en_US/FlashPlatform/reference/actionscript/3/flash/utils/ByteArray.html#readUTF()).

`Lmi` files contain a series of `LimbPic` and `Limb` AMF0 serialized objects each stored in an AMF3 ByteArray. They also contain the PNG files for character body parts each stored in an AMF3 ByteArray. In place of a PNG file can be just a null in case its corresponding `LimbPic` has the `disabled` value set to `true` and `embeded` set to `false`. The `LimbPic` and `Limb` series of objects are each prefixed with a 32-bit signed integer indicating their quantity, as indicated [here](https://help.adobe.com/en_US/FlashPlatform/reference/actionscript/3/flash/utils/ByteArray.html#readInt()). The `LimbPic`s come first, then the PNGs, then the `Limb`s.

_Note: HF's original `itemLmi` file has `disabled: false` and `embeded: true` for a LimbPic which doesn't have a PNG file associated._

`Spt` files contain just one `Spt` AMF0 serialized object stored in an AMF3 ByteArray. The `Bg` file contains one `BgInfoFile` AMF0 serialized object stored in an AMF3 ByteArray.

The `gdat` file contains a series of `Attack` and `PtWithName` AMF0 serialized objects each stored in an AMF3 ByteArray. The `Attack` and `PtWithName` series of objects are each prefixed with a 32-bit signed integer indicating their quantity, as indicated [here](https://help.adobe.com/en_US/FlashPlatform/reference/actionscript/3/flash/utils/ByteArray.html#readInt()). The `Attack`s come first, then the `PtWithName`s.

For Hero Fighter X, AMF0 is no longer used and the objects are serialized directly in AMF3 (also not inside AMF3 `ByteArray`s). The string indicating the file type also has an `O` at the end (e.g. `limbInfoO`).

[AMF0 file format](amf0-file-format-specification.pdf)  
[AMF3 file format](amf3-file-format-spec.pdf)


## Editing ActionScript

*TODO*

- [ActionScript Virtual Machine 2 (AVM2) Overview](avm2overview.pdf)
- [List of all AVM2 instructions](AVM2/AVM2 Instruction list.html)
- [Summary of the AVM2 instructions](AVM2/AVM2 Instructions.html)
- More links: [AVM2/JPEXS Free Flash Decompiler - Links.html](AVM2/JPEXS Free Flash Decompiler - Links.html)

What is **P-code**: [https://en.wikipedia.org/wiki/Bytecode](https://en.wikipedia.org/wiki/Bytecode)

## Compress / Decompress SWF

The SWF file structure<sup>[\[1\]](#swf-format)</sup> starts with a a signature which can be one of the following:

-   `FWS` - Uncompressed
-   `CWS` - zlib compressed
-   `ZWS` - LZMA compressed

The signature is followed by the [SWF version](https://www.adobe.com/devnet/air/articles/versioning-in-flash-runtime-swf-version.html).

### zlib

zlib compression works for SWF files of version >= 6. However, for SWF files with lower version it might work by simply changing the version byte to 6.

The algorithm to compress is as follows:

-   Copy the first 8 bytes of the SWF file which correspond to SWF file signature (3), version (1) and file length (4) to a buffer.
-   Change the 'F' in the signature to 'C'
-   Change the version to 6 if it is lower.
-   zlib compress the remaining bytes of the SWF file and add them to the buffer.

### LZMA

LZMA compression works for SWF files of version >= 13. However, for SWF files with lower version it might work by simply changing the version byte to 13.

LZMA compressed SWF files will work [in Flash Player 11 or AIR 3 or higher](https://www.adobe.com/devnet/flash/articles/concept-lzma-compression.html).

The algorithm to compress is as follows:

-   Copy the first 8 bytes of the SWF file which correspond to SWF file signature (3), version (1) and file length (4) to a buffer.
-   Change the 'F' in the signature to 'Z'
-   Change the version to 13 if it is lower.
-   LZMA compress the remaining bytes of the SWF file but don't add them to the buffer yet.
-   Calculate the length of the compressed bytes and subtract 5 to it (excluding LZMA properties<sup>[\[2\]](#lzma-format)</sup>).
-   Add the length calculated to the buffer in 4 little-endian bytes. Buffer should now have 12 bytes.
-   Add the compressed data to the buffer.

Note: Using XZ tools provides compression/decompression for formats .xz and .lzma. Both seem to not work with Adobe Flash Player. So I used LZMA SDK.

[Note from Adobe on LZMA vs 7z](https://helpx.adobe.com/flash-player/kb/exception-thrown-you-decompress-lzma-compressed.html)

<a name="swf-format"></a>*\[1]: swf-file-format-spec.pdf - Chapter 2*  \
<a name="lzma-format"></a>*\[2]: lzma-file-format.txt - Chapter 1.1*

Source: [https://github.com/OpenGG/swfzip/blob/master/swfzip.py](https://github.com/OpenGG/swfzip/blob/master/swfzip.py)

## SWF 2 EXE

Finding how to programmatically convert a .swf file to a .exe file was not straightforward. There are a few programs out there that do this (e.g. [swf2exe](https://sourceforge.net/projects/swf2exe/)), but they are apparently not open source. It was by chance that I was able to find [this](https://github.com/devil-tamachan/tama-ieeye/blob/master/SWF2EXE/swf2exe/swf2exe/MainDlg.h#L98) piece of code by [devil-tamachan](https://github.com/devil-tamachan) on GitHub that showed me how to do this programmatically.

The algorithm is quite simple to implement, but I don't know yet why it works like this. For starters, there is a software called [Adobe Flash Player projector](https://www.adobe.com/support/flashplayer/debug_downloads.html) (or Stand Alone), which is basically a program (.exe) that can run flash files (.swf). This program also has an option to create an .exe of the .swf file.

To convert programmatically, we do the following:

1.  Create a copy of the Flash Player projector.
2.  Append the .swf file to the projector's copy (merging the binaries basically).
3.  Append the following footer to the file: \[0x56, 0x34, 0x12, 0xFA]
4.  Append the size of the .swf file (4 bytes) to the file in little-endian format (less significant byte first). This can be the compressed length or uncompressed length, although the compressed length will allow us to calculate the start of the swf file afterwards.

There are different versions of the Flash Player projector, but this algorithm should work for all of them.

It is also possible to convert the SWF to a Linux binary, using the Adobe Flash Player projector for Linux. Programmatically it is slightly different than windows:

1.  Create a copy of the Flash Player projector.
2.  Append the size of the .swf file (4 bytes) to the file in little-endian format (less significant byte first). This can be the compressed length or uncompressed length, although the compressed length will allow us to calculate the start of the swf file afterwards.
3.  Append the following footer to the file: \[0x56, 0x34, 0x12, 0xFA]
4.  Append the .swf file to the projector's copy (merging the binaries basically).

_Note: The Flash Player for Linux 64-bit doesn't work with the above algorithm. It uses 8 bytes for the size of the swf and footer (which becomes \[0x56, 0x34, 0x12, 0xFA, 0xFF, 0xFF, 0xFF, 0xFF]), and it also requires some hex editing which varies depending on the FP version. More details can be found on [this project](https://github.com/shockpkg/swf-projector/issues/1)._

## EXE 2 SWF

This method assumes that the SWF is inside the executable and not encrypted, so it is a matter of detecting the start and the end of the SWF file in order to extract it.

1.  Detect if the file is a Windows executable, Linux executable, or not an executable.
    1.  Windows: The PE file's<sup>[\[1\]](#pe-file-format)</sup> magic number is `MZ`<sup>[\[2\]](#magic-number)</sup>, at offset 0x3C there is a 4 byte long integer that points to the beginning of the PE header, which has the signature `PE`.
    2.  Linux: The ELF file's signature is 0x7F followed by `ELF`<sup>[\[3\]](#elf-file)</sup>.
2.  We could search for the SWF signature, but there's no guarantee that there won't be multiple occurrences of this signature in the file. We could also check the length to make sure it reaches the end of the file where the footer FA123456 is found, although we could have a different executable that doesn't make use of this footer. To make things simple, we'll assume that we are dealing with the Flash Player projector and thus we have the footer FA123456 followed by the SWF file length (compressed) at the end of the file, in the case of Windows. In the case of Linux we have the footer FA123456 followed by the SWF and preceded by the SWF length

<a name="pe-file-format"></a>*\[1]: [PE file format image](https://upload.wikimedia.org/wikipedia/commons/1/1b/Portable_Executable_32_bit_Structure_in_SVG_fixed.svg) and [PE file format documentation](https://docs.microsoft.com/en-us/windows/desktop/debug/pe-format#machine-types)*  \
<a name="magic-number"></a>*\[2]: the initials of the designer of the file format, Mark Zbikowski: [Wikipedia - Magic Number](https://en.wikipedia.org/wiki/Magic_number_(programming))*  \
<a name="elf-file"></a>*\[3]: [Executable and Linkable Format](https://en.wikipedia.org/wiki/Executable_and_Linkable_Format)*

## Customizing the Flash Projector / HF executable

Adobe Flash Player projector can be downloaded [here](https://helpx.adobe.com/flash-player/kb/archived-flash-player-versions.html)

**Change icon** with  [Resource Editor](http://melander.dk/reseditor/) (.exe must not be compressed with UPX). And for changing the **window title**, it depends on the Flash Player version, some can be changed with Resource Hacker by editing the String table, and others can be changed only through hex editing (title may have null byte between characters). It is also possible to remove the **menu bar** with Resource Editor, by deleting the menu entries under the Menu folder.

Note: [Resource Hacker](http://www.angusj.com/resourcehacker/) appears to break Trong's Flash Player. [Resource Editor](http://melander.dk/reseditor/) is a good alternative that works.

Tip: On screens with high DPI, it is useful to go to exe's `Properties -> Compatibility -> Change high DPI settings` and tick `Use this setting to fix scaling problems for this program instead of the one in Settings` and also tick `Override high DPI scaling behavior. Scaling performed by: Application`. This will remove the blur from the app.

The Adobe Flash Player projector executable file is about 14-15 MiB in size. By using [UPX](https://upx.github.io/) we can decrease the size to about 5.5 MiB! We can unpack using `-d` flag.

[Article on UPX](https://labs.detectify.com/2016/04/12/using-reverse-engineering-techniques-to-see-how-a-common-malware-packer-works/)

*Note: SA stands for Standalone*

## Creating a modified APK

Changing the SWF file inside the original APK will cause the signature to no longer be valid. For this, we have to re-sign the APK, doing the following:

1. Remove the `META-INF` folder inside the APK fiile.
2. Create certificate:
   ```sh
   openssl req -newkey rsa:2048 -x509 -keyout hfxkey.pem -out hfx.x509.pem -days 10000
   # enter password
   openssl pkcs8 -topk8 -inform PEM -outform DER -nocrypt -in hfxkey.pem -out hfx.pk8
   ```
3. zipalign the APK:
   ```sh
   zipalign -v 4 HeroFighterX.apk HeroFighterX_aligned.apk
   rm HeroFighterX.apk && mv HeroFighterX_aligned.apk HeroFighterX.apk
   ```
4. Use `apksigner` (not `jarsigner`, because: https://stackoverflow.com/q/43006737/3049315)
   ```sh
   apksigner sign --key hfx.pk8 --cert hfx.x509.pem HeroFighterX.apk
   apksigner verify -v --print-certs HeroFighterX.apk
   rm HeroFighterX_aligned.apk
   ```
   > The private key file must use the PKCS #8 format, and the certificate file must use the X.509 format.  
   Reference: https://developer.android.com/studio/command-line/apksigner.html

For repackaging the APK using the latest Adobe AIR, see: https://stackoverflow.com/a/66080295/3049315