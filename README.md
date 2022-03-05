# sl1toctb - convert prusaslicer files to ctb v4 files

sl1toctb allows to convert files created by the prusaslicer 3d slicer into
chitu's ctb file, version 4, to use with any 3d printer supporting those
files.

Note that it will output ctb file version 4, not any earlier.

## Usage

```
sl1toctb [-h] [--help] <input sl1 file> [<output ctb file>] 
sl1toctb <-i> <ctb file> 
sl1toctb <-l|-s|-e> <dir> <ctb file> 
  
Convert a sl1 file into a ctb v4 file. 
Output file will have the same name as input file with the 
.sl1 extension replaced with .ctb, unless `output ctb file` is 
specified. 

When `-i` option is provided, inspect the given file instead.
This works with both ctb files and sl1 files.

When `-l` or `-s` option is provided, export preview image instead.
`-l` exports the large preview, and `-s` exports the small preview.
The file will be saved in `dir` and will be named `preview.png`.
Only works for ctb files.

When -e option is provided, export layers and their headers in `dir`.
Only works for ctb files.

Options: 

-i        : inspect file. (ctb or sl1) 
-l        : export large preview. (ctb only) 
-s        : export small preview. (ctb only) 
-e        : export layer data to <dir> (ctb only) 
-h|--help : show this help 
```

## Build

sl1toctb depends on:

* libzip
* zlib
* pkg-config
* make
* gcc

To build and install the software:

```
make
make install
```

It will install program sl1toctb in /usr/local/bin by default.

To install in an other directory, pass the PREFIX value directly to make:

```
make
make install PREFIX=/other/directory/
```

## Credits

[UVtools](https://github.com/sn4k3/UVtools) has been instrumental to my
understanding of the ctb file format, some parts of sl1toctb (notably the
RLE encoding/decoding and the encrypting/decrypting algorithms) are
basically line by line translations from UVtools. Thanks a lot to them! If
you want a multi-format, many features, actively developed tool, they're the
ones.

Thanks a lot also to [catibo](https://github.com/cbiffle/catibo)'s
developer, who did the initial reverse engineering of the ctb format
(version 2) and took the time to [properly document
it](https://github.com/cbiffle/catibo/blob/master/doc/cbddlp-ctb.adoc).
You rock.

## License

sl1toctb contains a copy of [libspng](https://libspng.org/), which is dual
licensed as BSD-2-Clause AND libpng-2.0. The rest of the code is public
domain.

## A word about maintenance

I hate maintenance. For real. Nothing is more annoying than to have to get
back on a project I though was done when I'm already on something else. So,
don't expect I'll be reactive in the issue tracker or I'll release new
features often (or even, probably: ever). I leave the issue tracker open,
though, so that people can discuss to solve each others problems. I will
probably merge pull request, or otherwise you can just fork the project,
that's something github is very good at.

It's the reason why this project is written in C, using only very stable
dependencies (libzip and zlib). The dependency I was not sure about its api
stability, libspng, has been vendor'd. So basically, sl1toctb should build
for the decades to come without a change. Or at the very least, for way
longer than the ctb v4 format will be used in the wild.

That's also why I put the code in public domain. If you have a problem with
this code, fix it and do whatever you want with it.
