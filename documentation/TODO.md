# Issues

- GNU Readline prompt on Windows cuts off characters when the line is too big.
- GNU Readline and Editline with unicode characters does not work.
- On Windows, unicode file names don't work unless the correct locale and chcp page are set.

# To-do

- Badges https://www.sonarqube.org/. See https://canary.discord.com/channels/@me/556927491240099870/914186266290049105
  
- AMF0 Arrays always use the "ECMA Array" regardless of indices being ordinal or not. This is inconvenient because adding and removing elements to an array in JSON requires changing the "Associate count" value as well. Also, it makes it differ from AMF3 array representations in JSON. One thing that can be done is verify if a key is an integer and add it to a "dense" portion and if not add it to the "associate" portion, as in AMF3 arrays. The problem with this approach is that then it is impossible to distinguish from ECMA Arrays and Strict Arrays in JSON. However, since Strict arrays are not being used by HF, there could be an optional flag in the `to_json` and `from_json` functions of AMF0 to treat every array as an ECMA Array.
  
- Convert AMF0 data structures to AMF3 data structures and vice-versa, so that JSONs can be converted as well.
  About saving int as number, AMF3 file spec says:

    > Note: this means the original type information of the data is potentially lost (though it 
    > may be possible to correct on deserialization when such values are assigned to strongly 
    > typed members of a class and coerced to a specified type). 

  Therefore, even if the type information is lost, the numbers can always be coerced to integers at runtime, if necessary. So, importing AMF0 JSON shouldn't be a problem as far as int vs floats are concerned.
  
  About sealed vs dynamic properties, AMF0 stores all  as same. The opposite is not true so I must check if there are any typed objects in HFX that use dynamic properties, and if yes, create a function in HFW that converts the AMF0 JSON to AMF3 JSON accordingly.

  About arrays, can check if indices of AMF0 ECMA Array are integers, and add them to the "dense portion" is yes, and to the "associative" portion if not. Since HF's AMF0 does not use the strict array, AMF3 arrays should always convert to ECMA arrays.
  
- optimize PNGs added by the user (make flag for disabling optimizations so that unit tests that compare files don't fail)

- When saving the file it will say "Permission denied" or something in case the EXE file that we're saving to is open. Mention that this might be the case in the error message.

- Make `1. Help` have 3 subsections: `1. README`, `2. About`, `3. Credits`

- Check if given APK is using Adobe AIR version 33+. If not, say it may not work on recent Android versions and give link to download HFX mod.

- Steps for re-signing apk: https://stackoverflow.com/a/15412477/3049315
  Source code of apksigner:
    - git clone https://android.googlesource.com/platform/tools/apksig

  Crypto++: https://cryptopp.com/
  A zipalign implementation:     
    - https://github.com/osm0sis/zipalign/blob/master/ZipAlign.cpp
    - https://github.com/mozilla-services/zipalign/blob/master/main.go
- Validate HF stories with XSD made by Nikhil Krishna. Review the XSD and credit Nikhil for making it. Look into http://xerces.apache.org/xerces-c/
- Editline introduced new bugs. Fix them. Wineditline (mingw) has different bugs.
    - Check out https://github.com/AmokHuginnsson/replxx. Did, doesn't work because: https://github.com/AmokHuginnsson/replxx/issues/103
    - Check compiling with CMake on Windows (wineditline)
    - Check compiling with Visual Studio
    
- use `uint8_t*` instead of `vector<uint8_t>` where possible. (And maybe `std::byte` instead of `uint8_t`)

- Coronation Wars 1.1 crashes HFW x86

- Compile for MSVC using appveyor.com

- ~~Add LGTM code quality tags: https://lgtm.com/~~ Doesn't work with GitLab repos

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

- HFX modes are encoded in ActionScript: https://canary.discord.com/channels/234364433344888832/605880341915238409/856471951592390666