#  md2teach

This repository is work-in-progress to try to use [md4c](https://github.com/mity/md4c) to create a markdown to Teach text converter ORCA shell program for the Apple //GS.  You should not try to use this tool because it doesn't work yet.

The goal really is to use this shell command from GoldenGate as part of my [Apple2GSBuildPipeline](https://github.com/jeremysrand/Apple2GSBuildPipeline) in order to write documentation for GS projects in a more modern file format.  Also, markdown is trivial to commit to git repositories but the presence of the resource fork in Teach files make them problematic.

The files md4c.c and md4c.h are (hopefully) slightly modified versions of the files from the md4c project.  It is shocking how portable that code was and it worked almost immediately.  I am marking up some of my changes with the comment tag GS_SPECIFIC.  In general, many unsigned types were converted to uint32_t and int types converted to int32_t.

For more information, review the [md2teach Readme from the SW distribution](/md2teach/Read.Me.md).

