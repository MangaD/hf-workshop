# Issues

- GNU Readline prompt on Windows cuts off characters when the line is too big.
- GNU Readline and Editline with unicode characters does not work.
- On Windows, unicode file names don't work unless the correct locale and chcp page are set.

# To-do

- HFX story code is perhaps encoded somehow. https://canary.discord.com/channels/234364433344888832/605880341915238409/856471951592390666
- HF v0.7 swf browser encrypted version does not come out equal! Same for downloadable SWF. It's wrecked by HFW. Investigate why.
- Make unit test that exports and replaces data, and checks if the uncompressed exported binary is equal to the uncompressed binary to replace.
- When saving the file it will say "Permission denied" or something in case the EXE file that we're saving to is open. Mention that this might be the case in the error message.
- Make `1. Help` have 3 subsections: `1. README`, `2. About`, `3. Credits`
- Validate HF stories with XSD made by Nikhil Krishna. Review the XSD and credit Nikhil for making it. Look into http://xerces.apache.org/xerces-c/
- make tests for libswf using HF_v0.3.0.swf
- optimize PNGs added by the user (optional so unit tests don't use it)
- Convert AMF0 JSON to AMF3 JSON and vice-versa. Convert data structures too if possible.
- Editline introduced new bugs. Fix them. Wineditline (mingw) has different bugs.
    - Check out https://github.com/AmokHuginnsson/replxx
    - Check compiling with CMake on Windows (wineditline)
    - Check compiling with Visual Studio
- use `uint8_t*` instead of `vector<uint8_t` where possible.
- Coronation Wars 1.1 crashes HFW x86
- Compile for MSVC using appveyor.com
- Add LGTM code quality tags: https://lgtm.com/
- Translate error messages.
- Add I/O API functions to `Contributing.md`.
- Export Linux 64 binary (requires hex editing). Check <https://github.com/shockpkg/swf-projector> and Discord chat with `JrMasterModelBuilder#5116`.
- `zlib_compress` loop can be simplified. Take a look here: <https://github.com/madler/zlib/blob/master/compress.c>
- Don't return a `std::vector` if you can help it instead take a buffer to write to and return the end pointer/how many bytes were written.
- Add JPEG and GIF support for replacing images.
- Print warning when changing version of SWF at compressing (possibly have 3 versions of this function, one that throws exeption, one that warns, one that ignores)
- Try compression with XZ
    - <https://sourceforge.net/p/lzmautils/discussion/708858/thread/cd04b6ace0/?limit=25#6050>
- Improve code
    - use array instead of vector for fixed size bytes
    - use forward iterator for `getTagsOfType` (<https://stackoverflow.com/questions/3582608/how-to-correctly-implement-custom-iterators-and-const-iterators>)
- Help option (only swf and portable exe work - official exe doesn't work (or does it? maybe compressed), mp3 and image files should be compressed, use UPX to compress projector). On Linux only Flash Player 11 projector works.
- Option to run program (execute directly from RAM)
- Make open source `libswf` with sample SWF, don't include Adobe Projector because of rights.
- Make it possible to edit SWF inside HFX APK.
- doxygen
- cppcheck
- Make a [JSON Schema](https://json-schema.org/) for data files to use with editors that support them. It gives autocompletion, checks for errors,...