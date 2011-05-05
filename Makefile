TARBALL := a6
TURNIN  := wacd.h wacd.c wac.h wac.c

PROGRAMS := wacd wac
SOURCES  := wacd.h wacd.c wac.h wac.c

CPPFLAGS :=
CFLAGS   :=
LDFLAGS  :=

include Rules.mk

wacd: wacd.o
	$(link_program)

wac: wac.o
	$(link_program)
