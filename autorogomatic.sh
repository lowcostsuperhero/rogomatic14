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
# If rogomatic itself fails (nonzero exit, e.g. an internal error), the
# failure is noted in $RGMDIR/autorogomatic.log and the next game is
# started.  The failed game still counts toward -r.  To avoid spinning
# when something is really broken, give up after MAXFAIL (default 10)
# failures in a row.
#
# environment: RGMDIR     directory holding rogomatic's score files, where wins
#                         are looked for (default: the installed rogomatic
#                         directory, normally /usr/local/share/rogomatic)
#              ROGOMATIC  rogomatic program (default ./rogomatic, else $PATH)
#              MAXFAIL    consecutive failures tolerated (default 10)

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

trap 'echo; echo "$NAME: interrupted after $n game(s)"; exit 130' INT TERM

n=0
before=$(wins)
while [[ $MAX -eq 0 || $n -lt $MAX ]]; do
    "$ROGOMATIC" "${HALF[@]}" -U "$RGM_U" -a "$RGM_A"
    rc=$?
    n=$((n + 1))
    # killed by the user's ^C / kill: stop, do not treat as a failure
    if [[ $rc -eq 130 || $rc -eq 143 ]]; then
        echo; echo "$NAME: interrupted after $n game(s)"
        exit $rc
    fi
    if [[ $rc -ne 0 ]]; then
        fails=$((fails + 1))
        totalfails=$((totalfails + 1))
        msg="rogomatic failed (exit $rc) on game $n; continuing"
        echo "$NAME: $msg" >&2
        echo "$(date '+%F %T') game $n: $msg" >> "$LOG"
        if [[ $fails -ge $MAXFAIL ]]; then
            echo "$NAME: $fails failures in a row, giving up" >&2
            exit $rc
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
