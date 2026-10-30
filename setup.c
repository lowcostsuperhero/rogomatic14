/*
 * setup.c: Rog-O-Matic XIV (CMU) Wed Jan 30 17:38:07 1985 - mlm
 * Copyright (C) 1985 by A. Appel, G. Jacobson, L. Hamey, and M. Mauldin
 *
 * This is the program which forks and execs the Rogue & the Player
 */

# include <stdio.h>
# include <signal.h>
# include <stdlib.h>
# include <fcntl.h>
# include <sys/ioctl.h>
# include <unistd.h>
# include <dirent.h>
# include <string.h>
# include <sys/stat.h>
# include "types.h"
# include "globals.h"
# include "install.h"

# define READ    0
# define WRITE   1

/*
 * find_in_path: look for an executable called name in each directory of
 * $PATH.  On success the full path is stored in buf and 1 is returned.
 */
static int find_in_path(const char *name, char *buf, size_t size)
{ const char *path = getenv ("PATH"), *end;
  size_t len;

  if (path == NULL) return 0;
  for (; *path; path = *end ? end + 1 : end)
  { end = path;
    while (*end && *end != ':') end++;
    len = (size_t) (end - path);
    if (len == 0) snprintf (buf, size, "./%s", name);
    else          snprintf (buf, size, "%.*s/%s", (int) len, path, name);
    if (access (buf, X_OK) == 0) return 1;
  }
  return 0;
}

/* Define the Rog-O-Matic pseudo-terminal (Concept Based) */

# define ROGUETERM "rg|rterm:am:bs:ce=^[^S:cl=^L:cm=^[a%+ %+ :co#80:li#24:so=^[D:se=^[d:pt:ta=^I:up=^[;:db:xn:"

int   rfrogue, rtrogue;

static void usage(FILE *f)
{ fprintf (f,
"Usage: rogomatic [options] [-f rogue] [-S seed] [-U usec] [-a secs]\n"
"       rogomatic -p [logfile]        replay a log file\n"
"       rogomatic [saved-game-file]\n"
"\n"
"  -a secs   seconds of silence from Rogue before rogomatic resynchronizes\n"
"            (default %g; 0 = never; fractions allowed)\n"
"  -c        cheat: use trap arrows\n"
"  -e        echo the game to the log file (roguelog)\n"
"  -E        Emacs mode\n"
"  -f file   use this Rogue program\n"
"  -h, -?    show this help and exit\n"
"  -H        no halftime show\n"
"  -p        play back the log file\n"
"  -r        restore a saved game\n"
"  -s        show the scores only\n"
"  -S seed   make Rogue build the same dungeon every time (no scoring)\n"
"  -t        terse: print status lines only\n"
"  -u        start in user mode\n"
"  -U usec   pause usec microseconds after each character sent to Rogue\n"
"            (default %d; 0 = full speed)\n"
"  -w        watched mode\n"
"\n"
"While playing: ^C enters interactive mode (? lists commands, Z shows the\n"
"seed, Q quits).\n", ROGTIMEOUT / 1000.0, (int) USLEEP);
}

/*
 * scoreversion: Which Rogue version's scores should "rogomatic -s" show?
 * Rogomatic keeps rgmscoreVERS / rgmdeltaVERS files in RGMDIR, one set per
 * Rogue version (for example 5.4.5).  Use the version of the most recently
 * changed one; with no score files yet, the Rogue version this branch plays.
 */

static char *scoreversion(void)
{ static char best[64] = "5.4.5";
  DIR *d = opendir (RGMDIR);
  struct dirent *e;
  time_t newest = 0;

  if (d == NULL) return (best);
  while ((e = readdir (d)) != NULL)
  { char *v = NULL, path[1024];
    struct stat sb;

    if (strncmp (e->d_name, "rgmscore", 8) == 0) v = e->d_name + 8;
    else if (strncmp (e->d_name, "rgmdelta", 8) == 0) v = e->d_name + 8;
    /* a version looks like 5.4.5: digits and dots, ending in a digit */
    if (v == NULL || *v < '0' || *v > '9' || strlen (v) >= sizeof best ||
        strspn (v, "0123456789.") != strlen (v) || v[strlen (v) - 1] == '.')
      continue;
    snprintf (path, sizeof path, "%s/%s", RGMDIR, e->d_name);
    if (stat (path, &sb) == 0 && sb.st_mtime >= newest)
    { newest = sb.st_mtime; strcpy (best, v); }
  }
  closedir (d);
  return (best);
}

int main(int argc, char *argv[])
{ int   ptc[2], ctp[2];
  int   child, score = 0, oldgame = 0;
  int   cheat = 0, noterm = 1, echo = 0, nohalf = 0, replay = 0;
  int   emacs = 0, rf = 0, terse = 0, user = 0, quitat = 2147483647;
  int   sf = 0, uf = 0;
  char  *usleeparg = "";
  long  usleep_usec = USLEEP, rogue_timeout_ms = ROGTIMEOUT;
  char  *timeoutarg = "";
  int   af = 0;
  char  *seedarg = "";
  char  *rfile = "", *rfilearg = "", options[96];
  char  ropts[256], roguename[128], rpath[1024];

  while (--argc > 0 && (*++argv)[0] == '-')
  { while (*++(*argv))
    { switch (**argv)
      { case 'c': cheat++;        break; /* Will use trap arrows! */
        case 'e': echo++;         break; /* Echo file to roguelog */
        case 'f': rf++;           break; /* Next arg is the rogue file */
        case 'S': sf++;           break; /* Next arg is the dungeon seed */
        case 'a': af++;           break; /* Next arg is the silence timeout */
        case 'U': uf++;           break; /* Next arg is usec between sends */
        case 'H': nohalf++;       break; /* No halftime show */
        case 'h': case '?':
                  usage (stdout); exit (0); /* Help */
        case 'p': replay++;       break; /* Play back roguelog */
        case 'r': oldgame++;      break; /* Use saved game */
        case 's': score++;        break; /* Give scores only */
        case 't': terse++;        break; /* Give status lines only */
        case 'u': user++;         break; /* Start up in user mode */
        case 'w': noterm = 0;     break; /* Watched mode */
        case 'E': emacs++;        break; /* Emacs mode */
        default:  usage (stderr);
                  exit (1);
      }
    }

    if (rf) 
    { if (--argc) rfilearg = *++argv;
      rf = 0;
    }
    if (sf)
    { if (--argc) seedarg = *++argv;
      sf = 0;
    }
    if (uf)
    { if (--argc) usleeparg = *++argv;
      uf = 0;
    }
    if (af)
    { if (--argc) timeoutarg = *++argv;
      af = 0;
    }
  }

  /* -a secs: seconds of silence from Rogue before resynchronizing */
  if (*timeoutarg)
  { char *end;
    double secs = strtod (timeoutarg, &end);
    if (*end || secs < 0 || secs > 3600)
    { fprintf (stderr, "rogomatic: -a needs seconds >= 0 (0 = never)\n");
      exit (1);
    }
    rogue_timeout_ms = (long) (secs * 1000.0 + 0.5);
  }

  /* -U usec: delay after each character sent, so the game can be watched */
  if (*usleeparg)
  { char *end;
    usleep_usec = strtol (usleeparg, &end, 0);
    if (*end || usleep_usec < 0)
    { fprintf (stderr, "rogomatic: -U needs an integer >= 0 (microseconds)\n");
      exit (1);
    }
  }

  /* -S seed: make Rogue build the same dungeon every time (and not score) */
  if (*seedarg)
  { char *end;
    unsigned long sv = strtoul (seedarg, &end, 0);
    if (*end) { fprintf (stderr, "rogomatic: bad seed '%s'\n", seedarg); exit (1); }
    { char sbuf[32];
      snprintf (sbuf, sizeof sbuf, "%lu", sv);
      setenv ("SEED", sbuf, 1);
    }
  }

  if (argc > 1)
  { usage (stderr);
    exit (1);
  }

  /* Find which rogue to use */
  if (*rfilearg)
  { if (access (rfilearg, 1) == 0)	rfile = rfilearg;
    else				{ perror (rfilearg); exit (1); }
  }
  else if (access ("rogue", 1) == 0)	rfile = "rogue";
# ifdef NEWROGUE
  else if (access (NEWROGUE, 1) == 0)	rfile = NEWROGUE;
# endif
# ifdef ROGUE
  else if (access (ROGUE, 1) == 0)	rfile = ROGUE;
# endif
  else if (find_in_path ("rogue", rpath, sizeof rpath))
					rfile = rpath;
  else
  { fprintf (stderr, "rogomatic: cannot find the rogue program (tried ./rogue"
# ifdef NEWROGUE
             ", " NEWROGUE
# endif
# ifdef ROGUE
             ", " ROGUE
# endif
             " and $PATH); use -f /path/to/rogue\n");
    exit (1);
  }

  if (!replay && !score) quitat = findscore (rfile, "Rog-O-Matic");

  sprintf (options, "%d,%d,%d,%d,%d,%d,%d,%d,%ld,%ld",
           cheat, noterm, echo, nohalf, emacs, terse, user,quitat,
           usleep_usec, rogue_timeout_ms);
  sprintf (roguename, "Rog-O-Matic %s for %s", RGMVER, getname ());
  sprintf (ropts, "name=%s,%s,%s,%s,%s,%s,%s,%s,%s,%s",
           roguename, "fruit=apricot", "terse", "noflush", "noask",
           "jump", "step", "nopassgo", "inven=slow", "seefloor");

  if (score)  { dumpscore (argc==1 ? argv[0] : scoreversion ()); exit (0); }
  if (replay) { replaylog (argc==1 ? argv[0] : ROGUELOG, options); exit (0); }

  if ((pipe (ptc) < 0) || (pipe (ctp) < 0))
  { fprintf (stderr, "Cannot get pipes!\n");
    exit (1);
  }

  rtrogue = ptc[WRITE];
  rfrogue = ctp[READ];

  if ((child = fork ()) == 0)
  { close (0);
    if (dup (ptc[READ]) < 0) _exit (1);
    close (1);
    if (dup (ctp[WRITE]) < 0) _exit (1);
    close (ptc[WRITE]); /* Close parent's (player's) unused end of the pipes */
    close (ctp[READ]);

    setenv ("TERMCAP", ROGUETERM, 1);
    setenv ("TERM", "rg", 1);
    /*
     * Modern curses reads terminfo, not termcap.  The "rg" entry (see
     * rterm.tic, compiled by "make terminfo") is looked for in ./terminfo
     * and then in RGMDIR/terminfo, unless TERMINFO is already set.
     */
    if (getenv ("TERMINFO") == NULL)
    { if (access ("terminfo", R_OK | X_OK) == 0)
         setenv ("TERMINFO", "terminfo", 1);
      else if (access (RGMDIR "/terminfo", R_OK | X_OK) == 0)
         setenv ("TERMINFO", RGMDIR "/terminfo", 1);
    }
    setenv ("ROGUEOPTS", ropts, 1);
    setenv ("LINES","24", 1);
    setenv ("COLUMNS","80", 1);
    signal (SIGWINCH, SIG_IGN);	/* rogue talks over pipes; ignore resizes */
    /*
     * Rogue talks over pipes, but its curses falls back to stderr to set
     * terminal modes.  Rogue calls raw(), which turns off ISIG on the
     * user's terminal (so ^C stops reaching rogomatic) and scrambles echo,
     * and puts the modes back at exit.  Keep Rogue off the terminal by
     * sending its stderr to a file instead.
     */
    { char errname[512]; int errfd;
      char *tmp = getenv ("TMPDIR");
      snprintf (errname, sizeof errname, "%s/rogomatic-rogue.%d.err",
                (tmp && *tmp) ? tmp : "/tmp", (int) getuid ());
      errfd = open (errname, O_WRONLY | O_CREAT | O_TRUNC | O_NOFOLLOW, 0600);
      if (errfd < 0) errfd = open ("/dev/null", O_WRONLY);
      if (errfd >= 0)
      { dup2 (errfd, 2);
        if (errfd > 2) close (errfd);
      }
    }
    signal (SIGINT, SIG_IGN);	/* ^C is for rogomatic, not for rogue */
    if (oldgame)  execl (rfile, rfile, "-r", NULL);
    if (argc)     execl (rfile, rfile, argv[0], NULL);
    execl (rfile, rfile, NULL);
    _exit (1);
  }

  else
  { /* Encode the open files into a two character string */

    char ft[3] = "aa", rp[32]; ft[0] += rfrogue; ft[1] += rtrogue;

    close (ptc[READ]); /* Close child's (rogue's) unused end of the pipes */
    close (ctp[WRITE]);

    /* Pass the process ID of the Rogue process as an ASCII string */
    sprintf (rp, "%d", child);

    if (!author () && nice (4) == -1) perror ("nice");

    execl ("player", "player", ft, rp, options, roguename, NULL);
# ifdef PLAYER
    execl (PLAYER, "player", ft, rp, options, roguename, NULL);
# endif
    execlp ("player", "player", ft, rp, options, roguename, NULL);
    printf ("Rogomatic not available, 'player' binary missing.\n");
    kill (child, SIGKILL);
  }
}

/* 
 * replaylog: Given a log file name and an options string, exec the player
 * process to replay the game.  No Rogue process is needed (since we are
 * replaying an old game), so the rfrogue and rtrogue file descrptiors are
 * given the fake value 'Z'.
 */

int replaylog(char *fname, char *options)
{ execl ("player", "player", "ZZ", "0", options, fname, NULL);
# ifdef PLAYER
  execl (PLAYER, "player", "ZZ", "0", options, fname, NULL);
# endif
  printf ("Replay not available, 'player' binary missing.\n");
  exit (1);
}

/*
 * author:
 *	See if a user is an author of the program
 */

int author(void)
{
  switch (getuid())
  { case 1337:	/* Fuzzy */
    case 1313:	/* Guy */
    case 1241:	/* Andrew */
    case 345:	/* Leonard */
    case 342:	/* Gordon */
		return 1;
    default:	return 0;
  }
}
