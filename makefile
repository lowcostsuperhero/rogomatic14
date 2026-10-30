# makefile: Rog-O-Matic XIV (CMU)
# Copyright (C) 1985 by A. Appel, G. Jacobson, L. Hamey, and M. Mauldin
#
# Modernized for current C compilers (gcc, clang; C99 through C23),
# ncurses, and flex.  Typical use:
#
#    make                        build everything
#    make install PREFIX=/usr/local
#
# Override CC, CFLAGS, LDFLAGS, LIBS, PREFIX or DESTDIR on the command
# line as needed.  The locations compiled into the programs may also
# be overridden, e.g.  make CPPFLAGS='-DROGUE=\"/usr/games/rogue\"'
#
CC        ?= cc
CFLAGS    ?= -O2 -g
CPPFLAGS  +=
LDFLAGS   +=
LEX       ?= flex
LIBS_CURSES = -lncurses
LIBS_MATH   = -lm

PREFIX    ?= /usr/local
RGMDIR    ?= $(PREFIX)/share/rogomatic
USLEEP    ?= 14000
BINDIR    ?= $(PREFIX)/bin
MANDIR    ?= $(PREFIX)/share/man/man6

# Locations compiled into the programs (see install.h).  Changing PREFIX,
# BINDIR or RGMDIR requires "make clean" first.
PATHDEFS  = -DUSLEEP=$(USLEEP) -DRGMDIR=\"$(RGMDIR)\" -DPLAYER=\"$(BINDIR)/player\" \
	-DROGUE=\"$(BINDIR)/rogue\" -DNEWROGUE=\"$(BINDIR)/rogue\"

BINARIES  = rogomatic player rgmplot datesub histplot gene

PLAYER_OBJS = arms.o command.o database.o debug.o explore.o io.o learn.o \
	ltm.o main.o mess.o monsters.o pack.o rand.o replay.o rooms.o \
	scorefile.o search.o stats.o strategy.o survival.o tactics.o \
	things.o titlepage.o utility.o worth.o
SRCS      = $(PLAYER_OBJS:.o=.c)
HDRS      = types.h globals.h protos.h install.h termtokens.h
OTHERS    = setup.c findscore.c datesub.l histplot.c rgmplot.c gene.c tf.c \
	rplot Bugreport

.PHONY: all clean install terminfo

all: $(BINARIES) terminfo

terminfo: rterm.tic
	tic -x -o terminfo rterm.tic
	touch terminfo

%.o: %.c $(HDRS)
	$(CC) $(PATHDEFS) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

player: $(PLAYER_OBJS)
	$(CC) $(LDFLAGS) -o $@ $(PLAYER_OBJS) $(LIBS_CURSES) $(LIBS_MATH)

rogomatic: setup.o findscore.o scorefile.o utility.o
	$(CC) $(LDFLAGS) -o $@ setup.o findscore.o scorefile.o utility.o

gene: gene.o rand.o learn.o stats.o utility.o
	$(CC) $(LDFLAGS) -o $@ gene.o rand.o learn.o stats.o utility.o \
		$(LIBS_MATH)

histplot: histplot.o utility.o
	$(CC) $(LDFLAGS) -o $@ histplot.o utility.o

rgmplot: rgmplot.o utility.o
	$(CC) $(LDFLAGS) -o $@ rgmplot.o utility.o

datesub.c: datesub.l
	$(LEX) -t datesub.l > $@

datesub.o: datesub.c
	$(CC) $(PATHDEFS) $(CPPFLAGS) $(CFLAGS) -Wno-unused-function -c -o $@ datesub.c

datesub: datesub.o
	$(CC) $(LDFLAGS) -o $@ datesub.o

tf: tf.o findscore.o utility.o
	$(CC) $(LDFLAGS) -o $@ tf.o findscore.o utility.o

clean:
	rm -rf *.o datesub.c core tf terminfo $(BINARIES)

install: all
	mkdir -p $(DESTDIR)$(BINDIR) $(DESTDIR)$(MANDIR)
	for b in $(BINARIES); do \
		install -m 755 $$b $(DESTDIR)$(BINDIR)/$$b; \
	done
	install -m 644 rogomatic.6 $(DESTDIR)$(MANDIR)/rogomatic.6
	mkdir -p $(DESTDIR)$(RGMDIR)
	chmod 1777 $(DESTDIR)$(RGMDIR)
	tic -x -o $(DESTDIR)$(RGMDIR)/terminfo rterm.tic
