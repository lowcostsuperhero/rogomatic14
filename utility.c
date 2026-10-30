/*
 * utility.c: Rog-O-Matic XIV (CMU) Tue Mar 26 15:27:03 1985 - mlm
 * Copyright (C) 1985 by A. Appel, G. Jacobson, L. Hamey, and M. Mauldin
 *
 * This file contains all of the miscellaneous system functions which
 * determine the baud rate, time of day, etc.
 *
 * If CMU is not defined, then various functions from libcmu.a are
 * defined here (otherwise the functions from -lcmu are used).
 */

# include <stdio.h>
# include <stdlib.h>
# include <stdarg.h>
# include <string.h>
# include <signal.h>
# include <pwd.h>
# include <unistd.h>
# include <fcntl.h>
# include <termios.h>
# include <sys/types.h>
# include <sys/stat.h>

# include "install.h"

# include <time.h>
# include <sys/time.h>

# define TRUE 1
# define FALSE 0

/*
 * baudrate: Determine the baud rate of the terminal
 */

int baudrate(void)
{ struct termios tio;
  speed_t sp;

  if (tcgetattr (fileno (stdin), &tio) != 0) return 9600;
  sp = cfgetospeed (&tio);
  switch (sp)
  { case B50: return 50;      case B75: return 75;
    case B110: return 110;    case B134: return 134;
    case B150: return 150;    case B200: return 200;
    case B300: return 300;    case B600: return 600;
    case B1200: return 1200;  case B1800: return 1800;
    case B2400: return 2400;  case B4800: return 4800;
    case B9600: return 9600;
    default: return sp == B0 ? 1200 : 9600;
  }
}


/*
 * getname: get userid of player.
 */

char *getname(void)
{ static char name[100] = "";
  struct passwd *pw;

  pw = getpwuid(getuid());

  if (pw != NULL)
      strncpy(name, pw->pw_name, 100);

  name[99] = 0;

  return (name);
}

/*
 * wopen: Open a file for world access.
 */

FILE *wopen(char *fname, char *mode)
{ int oldmask;
  FILE *newlog;

  oldmask = umask (0111);
  newlog = fopen (fname, mode);
  umask (oldmask);

  return (newlog);  
}

/*
 * fexists: return a boolean if the named file exists
 */

int fexists(char *fn)
{ struct stat pbuf;

  return (stat (fn, &pbuf) == 0);
}

/*
 * filelength: Do a stat and return the length of a file.
 */

int filelength(char *f)
{ struct stat sbuf;

  if (stat (f, &sbuf) == 0)
    return (sbuf.st_size);
  else
    return (-1);
}

/*
 * critical: Disable interrupts
 */

static void   (*hstat)(int), (*istat)(int), (*qstat)(int), (*pstat)(int);

void critical(void)
{
  hstat = signal (SIGHUP, SIG_IGN);
  istat = signal (SIGINT, SIG_IGN);
  pstat = signal (SIGPIPE, SIG_IGN);
  qstat = signal (SIGQUIT, SIG_IGN);
}

/*
 * uncritical: Enable interrupts
 */

void uncritical(void)
{
  signal (SIGHUP, hstat);
  signal (SIGINT, istat);
  signal (SIGPIPE, pstat);
  signal (SIGQUIT, qstat);
}

/*
 * reset_int: Set all interrupts to default
 */

void reset_int(void)
{
  signal (SIGHUP, SIG_DFL);
  signal (SIGINT, SIG_DFL);
  signal (SIGPIPE, SIG_DFL);
  signal (SIGQUIT, SIG_DFL);
}

/*
 * int_exit: Set up a function to call if we get an interrupt
 */

void int_exit(void (*exitproc)(int))
{
  if (signal (SIGHUP, SIG_IGN) != SIG_IGN)  signal (SIGHUP, exitproc);
  if (signal (SIGINT, SIG_IGN) != SIG_IGN)  signal (SIGINT, exitproc);
  if (signal (SIGPIPE, SIG_IGN) != SIG_IGN) signal (SIGPIPE, exitproc);
  if (signal (SIGQUIT, SIG_IGN) != SIG_IGN) signal (SIGQUIT, exitproc);
}

/*
 * lock_file: lock a file for a maximum number of seconds.
 *            Based on the method used in Rogue 5.2.
 */

# define NOWRITE 0

/* trylock: create the lock file; fails if it exists.  Closes the fd. */
static int trylock(char *lokfil)
{ int fd = open (lokfil, O_WRONLY | O_CREAT | O_EXCL, NOWRITE);
  if (fd < 0) return FALSE;
  close (fd);
  return TRUE;
}

int lock_file(char *lokfil, int maxtime)
{ int try;
  struct stat statbuf;

  start:
  if (trylock (lokfil))
    return TRUE;

  for (try = 0; try < 60; try++)
  { sleep (1);
    if (trylock (lokfil))
      return TRUE;
  }

  if (stat (lokfil, &statbuf) < 0)
  { (void) trylock (lokfil);
    return TRUE;
  }

  if (time (0) - statbuf.st_mtime > maxtime)
  { if (unlink (lokfil) < 0)
      return FALSE;
    goto start;
  }
  else
    return FALSE;
}

/*
 * unlock_file: Unlock a lock file.
 */

void unlock_file(char *lokfil)
{ (void) unlink (lokfil);
}

# ifndef CMU
/*
 * quit: Defined for compatibility with Berkeley 4.2 system
 */

/* VARARGS2 */
int quit (int code, char *fmt, ...)
{ va_list ap;
  va_start (ap, fmt);
  vfprintf (stderr, fmt, ap);
  va_end (ap);
  exit (code);
}

/*
 * stlmatch  --  match leftmost part of string
 *
 *  Usage:  i = stlmatch (big,small)
 *	int i;
 *	char *small, *big;
 *
 *  Returns 1 iff initial characters of big match small exactly;
 *  else 0.
 *
 *  HISTORY
 * 18-May-82 Michael Mauldin (mlm) at Carnegie-Mellon University
 *      Ripped out of CMU lib for Rog-O-Matic portability
 * 20-Nov-79  Steven Shafer (sas) at Carnegie-Mellon University
 *	Rewritten for VAX from Ken Greer's routine.
 *
 *  Originally from klg (Ken Greer) on IUS/SUS UNIX
 */

int stlmatch(char *big, char *small)
{ register char *s, *b;
  s = small;
  b = big;
  do
  { if (*s == '\0')
      return (1);
  }
  while (*s++ == *b++);
  return (0);
}
# endif

