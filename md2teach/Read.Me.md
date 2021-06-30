#  md2teach

The `md2teach` binary is a shell command for use under ORCA or GNO on an Apple //gs.  It is able to convert files from markdown format:

https://daringfireball.net/projects/markdown/syntax

To a Teach text file which can be opened on an Apple //gs.  This is *not* a GUI application for the GS and is really just a developer tool.

As a shell command, it can also be run from Golden Gate by Kelvin Sherlock:

https://juiced.gs/store/golden-gate/

This is the primary way that this is likely to be used.  Using Golden Gate, someone can run `md2teach` from their modern MacOS, Windows or Linux machine.

When doing cross development from a modern machine targeting the GS, if you want to write documentation for your application that can be opened on the GS, your options are a bit limited.  It would be nice to write your documentation in Teach text format.  That way, it can be opened on any GS and have nice formatting.  The problem is that the Teach text format encodes the style information in the resource fork of the file.  Modern systems in general may struggle with handling resource forks and even if they do handle them, tools like git for source code control often do not.

Using `md2teach`, you can write your documenation on your modern machine using markdown.  If you commit your code to an online repository like GitHub, then your markdown file can be viewed from a browser easily with text formatting in place.  At build time, you can run `md2teach` using Golden Gate and convert the markdown file to a Teach text file that you can include in your disk image and software distribution.

That way, modern systems have easy access to your documentation in a format that they can easily consume (markdown) and the GS can access your documentation using the Teach application, or other editors.

## Usage

You can use this shell command to convert from a markdown file called `input.md` to a file called `output` like this:

```
> md2teach input.md output
```

Note that the output is not sent to standard output but must go to a file.  Because it produces a file with a resource fork, it is not possible to send the output to standard output.

Similarly, from Golden Gate, you can run it like this:

```
> iix md2teach input.md output
```

Golden Gate did not originally support writing to resource forks using the Resource toolset.  Kelvin has made a beta version of Golden Gate available that has this capability called "rwrez-1".  Make sure you have this version of Golden Gate when using `md2teach` or you can use the `-r` argument (described below) to avoid directly writing to the resource fork from `md2teach` itself.

There are a few options you can specify:

* `-d` turns on debug output.  If you are having a problem with `md2teach`, it might be worth checking this debug output.  Or send the debug output to me with a description of your problem.
* `-v` prints out the version information for `md2teach`.
* `-r` turns on "Rez" mode.  Normally, the output of md2teach is a file with the text in the data fork and the style information in the resource fork.  In Rez mode, only the text is put in the output file and a second file with `.rez` appended to the file name is produced with the style information in a format that the resource compiler can read.  So, in the example above, if run in Rez mode, the text would be in a file called `output` and the style information will be in a file called `output.rez`.  If you are using an older version of Golden Gate, you may need to do this.  Then you can use the resource compiler to convert the `.rez` file to a resource fork and if you add that resource fork to the text file, you should end up with a file that Teach can load with the style information present.

## Links

If you are having any problems or have suggestions for `md2teach`, you can contact me at:

jeremy@rand-family.com

You can find the source code for `md2teach` on GitHub:

https://github.com/jeremysrand/md2teach

You can find all of my Apple // related projects on my website:

http://rand-emonium.com/

For markdown parsing, I ported md4c to the GS which you can find here:

https://github.com/mity/md4c

It is a very good, lightweight parser which made consuming markdown files pretty easy.

I hope you find `md2teach` useful.
