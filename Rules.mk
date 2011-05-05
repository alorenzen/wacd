# The user must define TARBALL, TURNIN, PROGRAMS, and SOURCES; if they define
# DEBUG, then debugging mode is turned on (its value is ignored).

################################## Functions ###################################

replace_only = $(patsubst $(1),$(2),$(filter $(1),$(3)))

objects      = $(call replace_only,%.c,%.o,$(1))
dependencies = $(call replace_only,%.c,$(DEPENDENCIES_DIR)/%.d,$(1))

link_program  = $(LINK.o) $^ $(LOADLIBES) $(LDLIBS) -o $@
build_library = $(AR) $(ARFLAGS) $@ $^

################################## Variables ###################################

# Configuration
DEPENDENCIES_DIR += dependencies
DEBUGFLAG_CFG    += -ggdb

# Debugging
ifdef DEBUG
DEBUGFLAG := $(DEBUGFLAG_CFG)
DEBUG     := -DDEBUG
endif

# Building
CC           := gcc
WARNFLAGS    += -Wall -pedantic -Werror
INCLUDEFLAGS += 
CPPFLAGS     += $(INCLUDEFLAGS) $(DEBUG)
CFLAGS       += -std=c99 $(DEBUGFLAG) $(WARNFLAGS)
LIBDIRFLAGS  += 
LDFLAGS      += $(DEBUGFLAG)
LDLIBS       += 

# Files
OBJECTS      = $(call objects,      $(SOURCES))
DEPENDENCIES = $(call dependencies, $(SOURCES))

################################ General rules #################################

.PHONY: all clean turnin

all: $(PROGRAMS)

clean:
	-rm -rf $(TARBALL).tar.gz $(PROGRAMS) $(OBJECTS) $(DEPENDENCIES_DIR)

$(TARBALL).tar.gz: $(TURNIN)
	mkdir $(TARBALL)
	ln $(TURNIN) $(TARBALL)
	tar czf $(TARBALL).tar.gz $(TARBALL)
	rm -rf $(TARBALL)
	turnin -c 432 $(TARBALL).tar.gz

turnin: $(TARBALL).tar.gz

################################ Pattern rules #################################

ifneq "$(MAKECMDGOALS)" "clean"
$(DEPENDENCIES_DIR)/%.d: %.c
	mkdir -p $(DEPENDENCIES_DIR)
	$(CC) -MM $(CPPFLAGS) $< | sed "s|$*.o|& $@|g" > $@

include $(DEPENDENCIES)
endif
