/*
 * protos.h: function prototypes for Rog-O-Matic XIV
 *
 * Included at the end of types.h.
 */

/* arms.c */
extern int havearmor(int k, int print, int rustproof);
extern int armorclass(int i);
extern int haveweapon(int k, int print);
extern int weaponclass(int i);
extern int havering(int k, int print);
extern int ringclass(int i);
extern int havebow(int k, int print);
extern int bowclass(int i);
extern int havemissile(void);
extern int havearrow(void);
extern void plusweapon(void);
extern int hitbonus(int strength);
extern int damagebonus(int strength);
extern void setbonuses(void);

/* command.c */
struct levstruct;
extern void command(int tmode, char *f, ...);
extern void move1(int d);
extern void fmove(int d);
extern void rmove(int count, int d, int mode);
extern void mmove(int d, int mode);
extern int commandcount(char *cmd);
extern char functionchar(char *cmd);
extern char commandarg(char *cmd, int n);
extern void adjustpack(char *cmd);
extern void bumpsearchcount(void);
extern int replaycommand(void);
extern void showcommand(char *cmd);
extern void clearcommand(void);
extern void usemsg(char *str, int obj);

/* database.c */
extern int findentry(stuff s, char *string);
extern void useobj(stuff s, char *oldname);
extern void infername(stuff s, int used, char *oldname, char *name);
extern int used(stuff s, char *oldname);
extern int know(stuff s, char *name);
extern char *realname(stuff s, char *oldname);
extern void dumpdatabase(void);

/* debug.c */
extern int dwait(int msgtype, char *f, ...);
extern void promptforflags(void);
extern void dumpflags(int r, int c);
extern void timehistory(FILE *f, int sep);
extern void toggledebug(void);
extern int getscrpos(char *msg, int *r, int *c);

/* explore.c */
extern int genericinit(void);
extern int gotowards(int r, int c, int running);
extern int gotoinit(void);
extern int gotovalue(int r, int c, int depth, int *val, int *avd, int *cont);
extern int sleepvalue(int r, int c, int depth, int *val, int *avd, int *cont);
extern int wallkind(int r, int c);
extern int setpsd(int print);
extern int downvalue(int r, int c, int depth, int *val, int *avd, int *cont);
extern int expruninit(void);
extern int exprunvalue(int r, int c, int depth, int *val, int *avd, int *cont);
extern int expunpininit(void);
extern int expunpinvalue(int r, int c, int depth, int *val, int *avd, int *cont);
extern int runinit(void);
extern int runvalue(int r, int c, int depth, int *val, int *avd, int *cont);
extern int unpininit(void);
extern int rundoorinit(void);
extern int rundoorvalue(int r, int c, int depth, int *val, int *avd, int *cont);
extern int expinit(void);
extern int roominit(void);
extern int expvalue(int r, int c, int depth, int *val, int *avd, int *cont);
extern int zigzagvalue(int r, int c, int depth, int *val, int *avd, int *cont);
extern int secretinit(void);
extern int secretvalue(int r, int c, int depth, int *val, int *avd, int *cont);
extern void avoidmonsters(void);
extern void caddycorner(int r, int c, int d1, int d2, int ch);
extern void pinavoid(void);
extern int secret(void);
extern int findroom(void);
extern int exploreroom(void);
extern int doorexplore(void);
extern int safevalue(int r, int c, int depth, int *val, int *avd, int *cont);
extern int findsafe(void);
extern int avoid(void);
extern int archmonster(int m, int trns);
extern int archeryinit(void);
extern int archeryvalue(int r, int c, int depth, int *val, int *avd, int *cont);
extern void unrest(void);
extern int movetorest(void);
extern int restinit(void);
extern int restvalue(int r, int c, int depth, int *val, int *avd, int *cont);

/* findscore.c */
extern int findscore(char *rogue, char *roguename);

/* histplot.c */
extern int getscore(int *score, char *killer, int *level);

/* io.c */
extern void getrogue(char *waitstr, int onat);
extern void terpbot(void);
extern void dumpwalls(void);
extern void sendcnow(int c);
extern int resend(void);
extern int pending(void);
extern int getroguetoken(void);
extern void sendnow(char *f, ...);
extern void send(char *f, ...);
extern void say(char *f, ...);
extern void saynow(char *f, ...);
extern void at(int r, int c);
extern void deadrogue(void);
extern void quitrogue(char *reason, int gld, int terminationtype);
extern void waitfor(char *mess);
extern void waitforspace(void);
extern void givehelp(void);
extern void pauserogue(void);
extern void getrogver(void);
extern int charsavail(void);
extern void redrawscreen(void);
extern void toggleecho(void);
extern void clearsendqueue(void);
extern void startreplay(FILE **logf, char *logfname);
extern void putn(char c, FILE *f, int n);
extern void printsnap(FILE *f);
extern int getlogtoken(void);
extern void getoldcommand(char *s);
extern void dosnapshot(void);
extern void clearscreen(void);
extern char *statusline(void);

/* learn.c */
extern void initpool(int k, int m);
extern void analyzepool(int full);
extern void setknobs(int *newid, int *knb, int *best, int *avg);
extern void evalknobs(int gid, int score, int level);
extern FILE *openlog(char *genelog);
extern void closelog(void);
extern int pickgenotype(void);
extern int readgenes(char *genepool);
extern void writegenes(char *genepool);

/* ltm.c */
extern void mapcharacter(int ch, char *str);
extern int addmonhist(char *monster);
extern int findmonster(char *monster);
extern void saveltm(int score);
extern void restoreltm(void);
extern void readltm(void);
extern void parsemonster(char *monster);
extern void clearltm(ltmrec *ltmarr);
extern void dumpmonstertable(void);
extern void analyzeltm(void);

/* main.c */
extern void onintr(int signum);
extern void startlesson(void);
extern void endlesson(void);

/* mess.c */
extern void terpmes(void);
extern void parsemsg(char *mess, char *mend);
extern int smatch(char *dat, char *pat, char **res);
extern void readident(char *name);
extern void dropreply(void);
extern void wieldreply(void);
extern void rampage(void);
extern void curseditem(void);
extern void infer(stuff s, char *objname);
extern void killed(char *monster);
extern void washit(char *monster);
extern void wasmissed(char *monster);
extern void didhit(void);
extern void didmiss(void);
extern void mshit(char *monster);
extern void msmiss(char *monster);
extern void countgold(char *amount);
extern void summary(FILE *f, int sep);
extern void versiondep(void);
extern int getmonhist(char *monster, int hitormiss);

/* monsters.c */
extern char *monname(char m);
extern void addmonster(int ch, int r, int c, int quiescence);
extern void deletemonster(int r, int c);
extern void dumpmonster(void);
extern void sleepmonster(void);
extern void holdmonsters(void);
extern void wakemonster(int dir);
extern int seemonster(char *monster);
extern int seeawakemonster(char *monster);
extern int monsternum(char *monster);
extern void newmonsterlevel(void);
extern int isholder(char *monster);

/* pack.c */
extern char *itemstr(int i);
extern void dumpinv(FILE *f);
extern void removeinv(int pos);
extern void deleteinv(int pos);
extern void clearpack(int pos);
extern void rollpackup(int pos);
extern void rollpackdown(int pos);
extern void resetinv(void);
extern void doresetinv(void);
extern int inventory(char *msgstart, char *msgend);
extern void countpack(void);

/* rand.c */
extern void rogo_srand(int seed);
extern int rogo_rand(void);
extern int randint(int max);

/* replay.c */
extern void positionreplay(void);
extern int findlevel(FILE *f, struct levstruct *lvpos, int *nmlev, int maxnum);
extern void fillstruct(FILE *f, struct levstruct *lev);
extern int findmatch(FILE *f, char *s);

/* rgmplot.c */
extern int getlin(char *s);

/* rooms.c */
extern void newlevel(void);
extern void markmissingrooms(void);
extern int whichroom(int r, int c);
extern void nametrap(int traptype, int standingonit);
extern void findstairs(int notr, int notc);
extern int downright(int *drow, int *dcol);
extern int lightroom(void);
extern int darkroom(void);
extern void currentrectangle(void);
extern void clearcurrect(void);
extern void updateat(void);
extern void updatepos(int ch, int row, int col);
extern void teleport(void);
extern void mapinfer(void);
extern void markexplored(int row, int col);
extern void unmarkexplored(int row, int col);
extern int isexplored(int row, int col);
extern int haveexplored(int n);
extern void printexplored(void);
extern void inferhall(int r, int c);
extern void connectdoors(int r1, int c1, int r2, int c2);
extern int canbedoor(int deadr, int deadc);
extern int mazedoor(int row, int col);
extern int nextto(int type, int r, int c);
extern int nexttowall(int r, int c);
extern void dumpmazedoor(void);
extern void foundnew(void);

/* scorefile.c */
extern void add_score(char *new_line, char *vers, int ntrm);
extern int dumpscore(char *vers);
extern void intrupscore(int signum);

/* search.c */
extern int makemove(int movetype, evalinit_fn evalinit, evaluate_fn evaluate, int reevaluate);
extern int findmove(int movetype, evalinit_fn evalinit, evaluate_fn evaluate, int reevaluate);
extern int followmap(int movetype);
extern int validatemap(int movetype, evalinit_fn evalinit, evaluate_fn evaluate);
extern void cancelmove(int movetype);
extern void setnewgoal(void);
extern int searchfrom(int row, int col, evaluate_fn evaluate, char dir[24][80], int *trow, int *tcol);
extern int searchto(int row, int col, evaluate_fn evaluate, char dir[24][80], int *trow, int *tcol);

/* setup.c */
extern int replaylog(char *fname, char *options);
extern int author(void);

/* stats.c */
extern void clearprob(probability *p);
extern void addprob(probability *p, int success);
extern double prob(probability *p);
extern void parseprob(char *buf, probability *p);
extern void writeprob(FILE *f, probability *p);
extern void clearstat(statistic * s);
extern void addstat(statistic *s, int datum);
extern double mean(statistic *s);
extern double stdev(statistic *s);
extern void parsestat(char *buf, statistic *s);
extern void writestat(FILE *f, statistic *s);

/* strategy.c */
extern int strategize(void);
extern int fightmonster(void);
extern int tomonster(void);
extern int wanttowake(char c);
extern int aftermelee(void);
extern int battlestations(int m, char *monster, int mbad, int danger, int mdir, int mdist, int alert, int adj);
extern int tostuff(void);
extern int fightinvisible(void);
extern int archery(void);
extern int pickupafter(void);
extern int dropjunk(void);
extern int quitforhonors(void);

/* survival.c */
extern int markcycles(int print);
extern void markchokepts(void);
extern int runaway(void);
extern int canrun(void);
extern int unpin(void);
extern int backtodoor(int dist);

/* tactics.c */
extern int handlearmor(void);
extern int handleweapon(void);
extern int quaffpotion(void);
extern int readscroll(void);
extern int handlering(void);
extern int findring(char *name);
extern int grope(int turns);
extern int findarrow(void);
extern int checkcango(int dir, int turns);
extern int godownstairs(int running);
extern int plunge(void);
extern int waitaround(void);
extern int goupstairs(int running);
extern int restup(void);
extern int gotowardsgoal(void);
extern int gotocorner(void);
extern int light(void);
extern int shootindark(void);
extern int dinnertime(void);
extern int trywand(void);
extern int eat(void);

/* things.c */
extern int wear(int obj);
extern int takeoff(void);
extern int wield(int obj);
extern int drop(int obj);
extern int quaff(int obj);
extern int reads(int obj);
extern int point(int obj, int dir);
extern int throw(int obj, int dir);
extern int puton(int obj);
extern int removering(int obj);
extern void initstufflist(void);
extern void addstuff(int ch, int row, int col);
extern void deletestuff(int row, int col);
extern void dumpstuff(void);
extern void display(char *s);
extern int prepareident(int obj, int iscroll);
extern int pickident(int iscroll);
extern int unknown(stuff otype);
extern int unidentified(stuff otype);
extern int haveother(stuff otype, int other);
extern int have(stuff otype);
extern int havenamed(stuff otype, char *name);
extern int havewand(char *name);
extern int wearing(char *name);
extern int havemult(stuff otype, char *name, int count);
extern int haveminus(void);
extern int haveuseless(void);
extern int willrust(int obj);
extern int wielding(stuff otype);
extern int hungry(void);
extern int weak(void);
extern int fainting(void);
extern int havefood(int n);

/* titlepage.c */
extern void animate(char *movie[]);
extern void halftimeshow(int level);

/* utility.c */
extern int baudrate(void);
extern int quit(int code, char *fmt, ...);
extern char *getname(void);
extern FILE *wopen(char *fname, char *mode);
extern int fexists(char *fn);
extern int filelength(char *f);
extern void critical(void);
extern void uncritical(void);
extern void reset_int(void);
extern void int_exit(void (*exitproc)(int));
extern int lock_file(char *lokfil, int maxtime);
extern void unlock_file(char *lokfil);
extern int stlmatch(char *big, char *small);

/* worth.c */
extern int worth(int obj);
extern int useless(int i);

