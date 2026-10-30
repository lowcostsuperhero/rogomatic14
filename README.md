# Rog-O-Matic XIV (modernized)

Rog-O-Matic, the program that plays Rogue, built for current systems with
gcc or clang (C99 through C23), ncurses and flex.  It drives a separate
`rogue` binary (see the companion rogue5.4 repository).  The original
documentation is in `README`.

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

### Compile-time paths

The locations built into the programs live in `install.h` and can be
overridden without editing it:

| Macro | Meaning | Default |
| --- | --- | --- |
| `RGMDIR` | data directory (score, gene pool, long-term memory) | `/usr/mlm/lib/rogomatic` |
| `PLAYER` | full path of the `player` binary | `/usr/mlm/bin/player` |
| `ROGUE` | full path of the `rogue` binary | `/usr/games/rogue` |
| `NEWROGUE` | preferred `rogue` binary, tried before `ROGUE` | `/usr/mlm/bin/rogue` |

For example:

```sh
make clean
make CPPFLAGS='-DRGMDIR=\"/var/games/rogomatic\" \
  -DPLAYER=\"/usr/local/bin/player\" -DROGUE=\"/usr/local/bin/rogue\"'
```

## Install

```sh
sudo make install PREFIX=/usr/local
```

Installs `rogomatic`, `player`, `rgmplot`, `datesub`, `histplot` and `gene`
into `$(PREFIX)/bin` (`BINDIR` to change) and `rogomatic.6` into
`$(PREFIX)/share/man/man6`.  Use `DESTDIR=/stage` for packaging.

Then create the data directory and install the terminal entry:

```sh
sudo mkdir -p /var/games/rogomatic            # your RGMDIR
sudo cp -r terminfo /var/games/rogomatic/
sudo chmod -R a+rwX /var/games/rogomatic      # or restrict to a games group
```

`rogomatic` looks for the terminal entry in `./terminfo` first, then in
`RGMDIR/terminfo`, unless `TERMINFO` is already set.

## Run

```sh
rogomatic                 # play, watching the screen
rogomatic -t              # terse: status lines only
rogomatic -f /path/rogue  # use a specific rogue binary
rogomatic -s              # show scores
```

See `rogomatic.6` (`man rogomatic`) for all options.

## Clean

```sh
make clean
```
