#!/usr/bin/env bash
#
# autorogomatic - play rogomatic over and over
#
# usage: autorogomatic [-r runs] [-s secs] [-a secs] [-U usecs] [-H]
#
#   -r runs   play at most this many games (default: forever)
#   -s secs   sleep this many seconds between games (default: 0), e.g. -s3
#             so that you can read the tombstone
#   -a secs   rogomatic -a value: timeout in seconds before rogomatic
#             decides rogue is stuck and forces a redraw (default 1)
#   -U usecs  rogomatic -U value: microseconds rogomatic sleeps after each
#             character it sends (default 1000)
#   -H        bring back the halftime show: do not pass -H to rogomatic
#             (by default autorogomatic passes -H, which disables it)
#
# Always stops early after a game that rogomatic wins.
#
# It does not stop for anything else: if rogomatic dies, crashes, or exits
# nonzero (the game ended in "Oops...", an internal error, a signal), the
# failure is noted in $RGMDIR/autorogomatic.log, the terminal is reset, and
# the next game is started.  A failed game still counts toward -r.  Only
# ^C that kills rogomatic as well, a kill of autorogomatic, -r games
# played, or a win ends the run.  After MAXFAIL (default 10) failures in a
# row it pauses 5 seconds between games so a truly broken setup does not
# spin the CPU, but it keeps going.
#
# environment: RGMDIR     directory holding rogomatic's score files, where wins
#                         are looked for (default: the installed rogomatic
#                         directory, normally /usr/local/share/rogomatic)
#              ROGOMATIC  rogomatic program (default ./rogomatic, else $PATH)
#              MAXFAIL    failures in a row before pausing (default 10)

NAME=${0##*/}
# "make install" fills in the real rogomatic directory here
DEFAULT_RGMDIR=@RGMDIR@
[[ $DEFAULT_RGMDIR == "@"* ]] && DEFAULT_RGMDIR=/usr/local/share/rogomatic
RGMDIR=${RGMDIR:-$DEFAULT_RGMDIR}
ROGOMATIC=${ROGOMATIC:-$(if [[ -x ./rogomatic ]]; then echo ./rogomatic; else type -P rogomatic; fi)}
MAX=0
SLEEP=0
RGM_A=1
RGM_U=1000
HALF=(-H)			# -H = no halftime show
MAXFAIL=${MAXFAIL:-10}
LOG=$RGMDIR/autorogomatic.log
fails=0
totalfails=0

usage() {
    echo "usage: $NAME [-r runs] [-s secs] [-a secs] [-U usecs] [-H]" >&2
    exit 2
}

while getopts ':r:s:a:U:H' opt; do
    case $opt in
    r) [[ $OPTARG =~ ^[1-9][0-9]*$ ]] || usage; MAX=$OPTARG ;;
    s) [[ $OPTARG =~ ^([0-9]+\.?[0-9]*|\.[0-9]+)$ ]] || usage; SLEEP=$OPTARG ;;
    a) [[ $OPTARG =~ ^([0-9]+\.?[0-9]*|\.[0-9]+)$ ]] || usage; RGM_A=$OPTARG ;;
    U) [[ $OPTARG =~ ^[0-9]+$ ]] || usage; RGM_U=$OPTARG ;;
    H) HALF=() ;;
    *) usage ;;
    esac
done
shift $((OPTIND - 1))
[[ $# -eq 0 ]] || usage

if [[ ! -x $ROGOMATIC ]]; then
    echo "$NAME: rogomatic not found" >&2
    exit 2
fi
mkdir -p "$RGMDIR" 2>/dev/null
[[ -d $RGMDIR && -w $RGMDIR ]] || { echo "$NAME: cannot use $RGMDIR (set RGMDIR)" >&2; exit 2; }

# number of won games recorded in the score files ("total winner")
wins() {
    cat "$RGMDIR"/rgmscore* "$RGMDIR"/rgmdelta* 2>/dev/null | grep -c 'total winner'
}

# ^C or kill aimed at this script ends the run; the same signal reaching
# rogomatic (which handles ^C itself) must not.
# A ^C typed at the terminal reaches rogomatic too, and rogomatic answers it
# by going interactive and carrying on, so ^C only ends the run if rogomatic
# died of it as well.
stop=0 intr=0
trap 'intr=1' INT
trap 'stop=1' TERM

n=0
before=$(wins)
while [[ $MAX -eq 0 || $n -lt $MAX ]]; do
    "$ROGOMATIC" "${HALF[@]}" -U "$RGM_U" -a "$RGM_A"
    rc=$?
    n=$((n + 1))
    [[ $rc -lt 128 ]] && intr=0
    # the user interrupted this script: stop, this is not a failure
    if [[ $stop -ne 0 || ( $intr -ne 0 && $rc -ge 128 ) ]]; then
        echo; echo "$NAME: interrupted after $n game(s)"
        exit 130
    fi
    if [[ $rc -ne 0 ]]; then
        fails=$((fails + 1))
        totalfails=$((totalfails + 1))
        msg="rogomatic failed (exit $rc) on game $n; continuing"
        echo "$NAME: $msg" >&2
        echo "$(date '+%F %T') game $n: $msg" >> "$LOG"
        # rogomatic may have died with the terminal in an odd mode
        [[ -t 0 ]] && stty sane 2>/dev/null
        if [[ $fails -ge $MAXFAIL ]]; then
            echo "$NAME: $fails failures in a row, pausing 5 seconds" >&2
            sleep 5
        fi
    else
        fails=0
    fi
    if [[ $(wins) -gt $before ]]; then
        echo "$NAME: rogomatic WON on game $n!"
        exit 0
    fi
    # not the last game: say which run just ended, then pause before the next
    if [[ $MAX -eq 0 || $n -lt $MAX ]]; then
        if [[ $MAX -eq 0 ]]; then
            echo "Run $n"
        else
            echo "Run $n of $MAX"
        fi
        sleep "$SLEEP"
    fi
done
echo "$NAME: played $n game(s), no win ($totalfails failed)"
