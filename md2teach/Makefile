#
#  Makefile
#  	Apple //GS Build Engine for ORCA and Merlin
#

include make/head.mk

DISKIMAGE=$(TARGETDIR)/md2teach.2mg
ARCHIVE=$(TARGETDIR)/md2teach.shk
DESTBOOTIMAGE=$(TARGETDIR)/$(BOOTIMAGE)

MAMEARGS=apple2gs -skip_gameinfo -speed 3 -mouse -window -resolution 1408x1056 -ramsize 4M -sl7 cffa202
EMULATOR=mame

gen:

build:
	make -f shell.mk
	make -f babelfish.mk
	
clean:
	make -f shell.mk clean
	make -f babelfish.mk clean
	
execute:
	make -f shell.mk execute
	make/createDiskImage "$(DISKIMAGE)" $(DESTBOOTIMAGE) "$(TARGETDIR)/Markdown" "$(TARGETDIR)/md2teach" "$(TARGETDIR)/outfile.txt" test.md
	make/launchEmulator "$(DISKIMAGE)" "$(DESTBOOTIMAGE)"
