# Rog-O-Matic XIV (modernized)

Rog-O-Matic, the program that plays Rogue, built for current systems with
gcc or clang (C99 through C23), ncurses and flex.  It drives a separate
`rogue` binary (see the companion rogue5.4 repository).  The original
documentation is in `README`.

## Heritage

This is a modernized continuation of Rog-O-Matic XIV, derived from
[rogueforge/rogomatic14](https://github.com/rogueforge/rogomatic14)
(snapshot 06e103a3c2c11acc6c4351f650d6cd137cb59aac).  It is not a GitHub
fork, but the original authors' copyright and licence (`COPYING`,
`LICENSE`) apply unchanged.  It plays the Rogue built from the companion
[rogue5.4](https://github.com/lowcostsuperhero/rogue5.4) repository.


## Prerequisites

- `gcc` or `clang`, `make`
- ncurses development files and `tic` (terminfo compiler)
- `flex` (for `datesub`)
- A `rogue` executable (build it from the rogue5.4 repository)

RHEL / Oracle / Alma / Rocky:
`sudo dnf install gcc make ncurses-devel flex`
Debian / Ubuntu: `sudo apt install gcc make libncurses-dev flex`

## Compile

```sh
make
```

This builds all of the following in the source directory:

| Binary | Purpose |
| --- | --- |
| `rogomatic` | launcher: starts Rogue and the player, handles scores and replays |
| `player` | the playing program itself (started by `rogomatic`) |
| `rgmplot` | plots score data |
| `histplot` | histogram plots of score data |
| `gene` | genetic-algorithm gene pool utility |
| `datesub` | filter that converts month names to numbers (built with flex) |

It also compiles `rterm.tic` into `./terminfo`, the `rg` terminal that
Rogue is run under.  `make tf` builds the optional `tf` score-finder test
tool.  Build one program with e.g. `make player`.

Override tools and flags on the command line:

```sh
make CC=clang CFLAGS='-O2 -g'
```

### Install locations

The locations built into the programs follow `PREFIX` (default
`/usr/local`):

| Make variable | Meaning | Default |
| --- | --- | --- |
| `BINDIR` | where `rogomatic`, `player` and `rogue` are found | `$(PREFIX)/bin` |
| `RGMDIR` | data directory (scores, gene pool, long-term memory, terminfo) | `$(PREFIX)/share/rogomatic` |

`rogomatic` looks for Rogue in this order: the `-f` argument, `./rogue`,
`$(BINDIR)/rogue`, then each directory of `$PATH`.  It finds `player` in
`./player`, `$(BINDIR)/player`, then `$PATH`.  If you change `PREFIX`,
`BINDIR` or `RGMDIR`, run `make clean` before `make`:

```sh
make clean
make PREFIX=/opt/games RGMDIR=/var/games/rogomatic
```

The names can also be set directly in `install.h`.

## Install

```sh
sudo make install PREFIX=/usr/local      # same PREFIX/RGMDIR as used to build
```

Installs `rogomatic`, `player`, `rgmplot`, `datesub`, `histplot`, `gene` and
the `autorogomatic` script into `$(BINDIR)`, `rogomatic.6` into `$(PREFIX)/share/man/man6`, and creates
the world-writable (sticky) data directory `RGMDIR` containing the `rg`
terminfo entry.  Use `DESTDIR=/stage` for packaging.  Rogue itself must be
installed separately (see the rogue5.4 repository); if it is not in
`$(BINDIR)` or on `$PATH`, run `rogomatic -f /path/to/rogue`.

## Run

```sh
rogomatic                 # play, watching the screen
rogomatic -t              # terse: status lines only
rogomatic -f /path/rogue  # use a specific rogue binary
rogomatic -s              # show scores
```

See `rogomatic.6` (`man rogomatic`) for all options.

### Playing many games: autorogomatic

```sh
autorogomatic                  # play game after game until a win (^C stops)
autorogomatic -r 20 -s 3       # at most 20 games, 3 seconds between games
autorogomatic -H               # bring back the halftime show
autorogomatic -U 0 -a 2        # full speed, 2 s silence timeout
```

It runs `rogomatic -H -U 1000 -a 1` repeatedly: `-U` is the pause in
microseconds after each character sent to Rogue, `-a` is the seconds of
silence from Rogue before rogomatic resynchronizes.  `autorogomatic -H`
leaves out rogomatic's `-H`, so the halftime show plays.  It stops after a
win or after the `-r` games, and never gives up on failed games (it logs
them to `RGMDIR/autorogomatic.log` and plays on).  Wins are looked for in
the score files in `RGMDIR` (set the `RGMDIR` environment variable if the
data directory is not the installed one); `ROGOMATIC` selects the
rogomatic program.

## Reporting a freeze (finding the game seed)

Rogue's error output goes to `$TMPDIR/rogomatic-rogue.UID.err` (`/tmp` if
TMPDIR is unset) so that Rogue cannot change your terminal modes; look
there if Rogue fails to start.

If Rogue stays silent for 1 second, rogomatic assumes both programs are
waiting on each other and resynchronizes (this is the brief pause you may see
now and then).  `rogomatic -a secs` changes the wait (fractions are fine;
`-a 0` never gives up waiting; `make ROGTIMEOUT=ms` changes the default).

`rogomatic -h` lists all options (the old no-halftime flag is now `-H`).

Rogomatic pauses 14 ms after each character it sends to Rogue so the game
can be watched.  `rogomatic -U usec` changes that pause in microseconds
(`-U 0` is full speed; `make USLEEP=...` changes the default).

`rogomatic -S seed` makes Rogue build the same dungeon every time, so a
freeze can be reproduced.  (Rogue does not record scores for seeded games.)
Try different seeds in a loop to find one that freezes:

```sh
for s in $(seq 1 200); do timeout 120 rogomatic -S $s >/dev/null || echo "seed $s"; done
```

Press `^C` to drop into interactive mode, then `Z` to show the seed (exact
with `-S`, otherwise start time + Rogue pid, give or take a second).

If a game without `-S` freezes, its seed can still be recovered while it is
stuck.  Rogue's seed is the start time (seconds since 1970) plus Rogue's
process id.  Either read it from the running Rogue with gdb:

```sh
gdb -p $(pgrep -n rogue) -batch -ex 'print dnum'
```

or compute it:  `echo $(( $(date -d "$(ps -o lstart= -p ROGUEPID)" +%s) + ROGUEPID ))`
(it can be off by a second; try the neighbouring values).  Also useful for
a report is what the stuck `player` is doing:

```sh
top -b -n1 | grep -E 'player|rogue'      # 100% CPU = busy loop, 0% = waiting on a pipe
gdb -p $(pgrep -n player) -batch -ex bt  # where it is stuck
```

and the last line shown at the top of the screen.

## Clean

```sh
make clean
```
