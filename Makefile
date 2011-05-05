TARBALL := a6
TURNIN  := wacd.h wacd.c wac.h wac.c

PROGRAMS := wacd standard silly
SOURCES  := wacd.c wac.c standard.c silly.c

CPPFLAGS := -D_BSD_SOURCE -DWACD_PHYSICAL
CFLAGS   :=
LDFLAGS  :=

include Rules.mk

wacd: wacd.o
	$(link_program)

standard: standard.o wac.o
	$(link_program)

silly: silly.o wac.o
	$(link_program)
