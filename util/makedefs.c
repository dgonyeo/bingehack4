/*	SCCS Id: @(#)makedefs.c	3.4	2002/08/14	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* Copyright (c) M. Stephenson, 1990, 1991.			  */
/* Copyright (c) Dean Luick, 1990.				  */
/* NetHack may be freely redistributed.  See license for details. */

#define MAKEDEFS_C	/* use to conditionally include file sections */
/* #define DEBUG */	/* uncomment for debugging info */

#include "config.h"
#include "permonst.h"
#include "objclass.h"
#include "monsym.h"
#include "artilist.h"
#include "dungeon.h"
#include "obj.h"
#include "monst.h"
#include "you.h"
#include "flag.h"
#include "dlb.h"

/* version information */
#ifdef SHORT_FILENAMES
#include "patchlev.h"
#else
#include "patchlevel.h"
#endif

#define Fprintf	(void) fprintf
#define Fclose	(void) fclose
#define Unlink	(void) unlink
#define rewind(fp) fseek((fp),0L,SEEK_SET)	/* guarantee a return value */

#if defined(UNIX) && !defined(LINT) && !defined(GCC_WARN)
static	const char	SCCS_Id[] = "@(#)makedefs.c\t3.4\t2002/02/03";
#endif

	/* names of files to be generated */
#define DATE_FILE	"date.h"
#define MONST_FILE	"pm.h"
#define ONAME_FILE	"onames.h"
#ifndef OPTIONS_FILE
#define OPTIONS_FILE	"options"
#endif
#define ORACLE_FILE	"oracles"
#define DATA_FILE	"data"
#define RUMOR_FILE	"rumors"
#define DGN_I_FILE	"dungeon.def"
#define DGN_O_FILE	"dungeon.pdf"
#define MON_STR_C	"monstr.c"
#define QTXT_I_FILE	"quest.txt"
#define QTXT_O_FILE	"quest.dat"
	/* locations for those files */
#define INCLUDE_TEMPLATE	"../include/%s"
#define SOURCE_TEMPLATE		"../src/%s"
#define DGN_TEMPLATE		"../dat/%s"  /* where dungeon.pdf file goes */
#define DATA_TEMPLATE		"../dat/%s"
#define DATA_IN_TEMPLATE	"../dat/%s"

static const char
    *Dont_Edit_Code =
	"/* This source file is generated by 'makedefs'.  Do not edit. */\n",
    *Dont_Edit_Data =
	"#\tThis data file is generated by 'makedefs'.  Do not edit. \n";

static struct version_info version;


static char	in_line[256], filename[60];

#ifdef FILE_PREFIX
		/* if defined, a first argument not starting with - is
		 * taken as a text string to be prepended to any
		 * output filename generated */
char *file_prefix="";
#endif

int FDECL(main, (int,char **));
void FDECL(do_makedefs, (char *));
void NDECL(do_objs);
void NDECL(do_data);
void NDECL(do_dungeon);
void NDECL(do_date);
void NDECL(do_options);
void NDECL(do_monstr);
void NDECL(do_permonst);
void NDECL(do_questtxt);
void NDECL(do_rumors);
void NDECL(do_oracles);

extern void NDECL(monst_init);		/* monst.c */
extern void NDECL(objects_init);	/* objects.c */

static void NDECL(make_version);
static char *FDECL(version_string, (char *));
static char *FDECL(version_id_string, (char *,const char *));
static char *FDECL(xcrypt, (const char *));
static int FDECL(check_control, (char *));
static char *FDECL(without_control, (char *));
static boolean FDECL(d_filter, (char *));
static boolean FDECL(h_filter, (char *));
static boolean FDECL(ranged_attk,(struct permonst*));
static int FDECL(mstrength,(struct permonst *));
static void NDECL(build_savebones_compat_string);

static boolean FDECL(qt_comment, (char *));
static boolean FDECL(qt_control, (char *));
static int FDECL(get_hdr, (char *));
static boolean FDECL(new_id, (char *));
static boolean FDECL(known_msg, (int,int));
static void FDECL(new_msg, (char *,int,int));
static void FDECL(do_qt_control, (char *));
static void FDECL(do_qt_text, (char *));
static void NDECL(adjust_qt_hdrs);
static void NDECL(put_qt_hdrs);

static char *FDECL(tmpdup, (const char *));
static char *FDECL(limit, (char *,int));
static char *FDECL(eos, (char *));

/* input, output, tmp */
static FILE *ifp, *ofp, *tfp;


int
main(argc, argv)
int	argc;
char	*argv[];
{
	if ( (argc != 2)
#ifdef FILE_PREFIX
		&& (argc != 3)
#endif
	) {
	    Fprintf(stderr, "Bad arg count (%d).\n", argc-1);
	    (void) fflush(stderr);
	    return 1;
	}

#ifdef FILE_PREFIX
	if(argc >=2 && argv[1][0]!='-'){
	    file_prefix=argv[1];
	    argc--;argv++;
	}
#endif
	do_makedefs(&argv[1][1]);
	exit(EXIT_SUCCESS);
	/*NOTREACHED*/
	return 0;
}

void
do_makedefs(options)
char	*options;
{
	boolean more_than_one;

	/* Note:  these initializers don't do anything except guarantee that
		we're linked properly.
	*/
	monst_init();
	objects_init();

	/* construct the current version number */
	make_version();


	more_than_one = strlen(options) > 1;
	while (*options) {
	    if (more_than_one)
		Fprintf(stderr, "makedefs -%c\n", *options);

	    switch (*options) {
		case 'o':
		case 'O':	do_objs();
				break;
		case 'd':
		case 'D':	do_data();
				break;
		case 'e':
		case 'E':	do_dungeon();
				break;
		case 'm':
		case 'M':	do_monstr();
				break;
		case 'v':
		case 'V':	do_date();
				do_options();
				break;
		case 'p':
		case 'P':	do_permonst();
				break;
		case 'q':
		case 'Q':	do_questtxt();
				break;
		case 'r':
		case 'R':	do_rumors();
				break;
		case 'h':
		case 'H':	do_oracles();
				break;

		default:	Fprintf(stderr,	"Unknown option '%c'.\n",
					*options);
				(void) fflush(stderr);
				exit(EXIT_FAILURE);
		
	    }
	    options++;
	}
	if (more_than_one) Fprintf(stderr, "Completed.\n");	/* feedback */

}


/* trivial text encryption routine which can't be broken with `tr' */
static
char *xcrypt(str)
const char *str;
{				/* duplicated in src/hacklib.c */
	static char buf[BUFSZ];
	register const char *p;
	register char *q;
	register int bitmask;

	for (bitmask = 1, p = str, q = buf; *p; q++) {
		*q = *p++;
		if (*q & (32|64)) *q ^= bitmask;
		if ((bitmask <<= 1) >= 32) bitmask = 1;
	}
	*q = '\0';
	return buf;
}

void
do_rumors()
{
	char	infile[60];
	long	true_rumor_size;

	filename[0]='\0';
#ifdef FILE_PREFIX
	Strcat(filename,file_prefix);
#endif
	Sprintf(eos(filename), DATA_TEMPLATE, RUMOR_FILE);
	if (!(ofp = fopen(filename, WRTMODE))) {
		perror(filename);
		exit(EXIT_FAILURE);
	}
	Fprintf(ofp,Dont_Edit_Data);

	Sprintf(infile, DATA_IN_TEMPLATE, RUMOR_FILE);
	Strcat(infile, ".tru");
	if (!(ifp = fopen(infile, RDTMODE))) {
		perror(infile);
		Fclose(ofp);
		Unlink(filename);	/* kill empty output file */
		exit(EXIT_FAILURE);
	}

	/* get size of true rumors file */
	(void) fseek(ifp, 0L, SEEK_END);
	true_rumor_size = ftell(ifp);
	Fprintf(ofp,"%06lx\n", true_rumor_size);
	(void) fseek(ifp, 0L, SEEK_SET);

	/* copy true rumors */
	while (fgets(in_line, sizeof in_line, ifp) != 0)
		(void) fputs(xcrypt(in_line), ofp);

	Fclose(ifp);

	Sprintf(infile, DATA_IN_TEMPLATE, RUMOR_FILE);
	Strcat(infile, ".fal");
	if (!(ifp = fopen(infile, RDTMODE))) {
		perror(infile);
		Fclose(ofp);
		Unlink(filename);	/* kill incomplete output file */
		exit(EXIT_FAILURE);
	}

	/* copy false rumors */
	while (fgets(in_line, sizeof in_line, ifp) != 0)
		(void) fputs(xcrypt(in_line), ofp);

	Fclose(ifp);
	Fclose(ofp);
	return;
}

/*
 * 3.4.1: way back in 3.2.1 `flags.nap' became unconditional but
 * TIMED_DELAY was erroneously left in VERSION_FEATURES and has
 * been there up through 3.4.0.  Simply removing it now would
 * break save file compatibility with 3.4.0 files, so we will
 * explicitly mask it out during version checks.
 * This should go away in the next version update.
 */
#define IGNORED_FEATURES	( 0L \
				| (1L << 23)	/* TIMED_DELAY */ \
				)

static void
make_version()
{
	register int i;

	/*
	 * integer version number
	 */
	version.incarnation = ((unsigned long)VERSION_MAJOR << 24) |
				((unsigned long)VERSION_MINOR << 16) |
				((unsigned long)PATCHLEVEL << 8) |
				((unsigned long)EDITLEVEL);
	/*
	 * encoded feature list
	 * Note:  if any of these magic numbers are changed or reassigned,
	 * EDITLEVEL in patchlevel.h should be incremented at the same time.
	 * The actual values have no special meaning, and the category
	 * groupings are just for convenience.
	 */
	version.feature_set = (unsigned long)(0L
		/* levels and/or topology (0..4) */
#ifdef REINCARNATION
			| (1L <<  1)
#endif
#ifdef SINKS
			| (1L <<  2)
#endif
		/* monsters (5..9) */
#ifdef KOPS
			| (1L <<  6)
#endif
		/* objects (10..14) */
#ifdef TOURIST
			| (1L << 10)
#endif
#ifdef STEED
			| (1L << 11)
#endif
#ifdef GOLDOBJ
			| (1L << 12)
#endif
		/* flag bits and/or other global variables (15..26) */
#ifdef TEXTCOLOR
			| (1L << 17)
#endif
#ifdef INSURANCE
			| (1L << 18)
#endif
#ifdef ELBERETH
			| (1L << 19)
#endif
#ifdef EXP_ON_BOTL
			| (1L << 20)
#endif
#ifdef SCORE_ON_BOTL
			| (1L << 21)
#endif
		/* data format [COMPRESS excluded] (27..31) */
#ifdef ZEROCOMP
			| (1L << 27)
#endif
#ifdef RLECOMP
			| (1L << 28)
#endif
			);
	/*
	 * Value used for object & monster sanity check.
	 *    (NROFARTIFACTS<<24) | (NUM_OBJECTS<<12) | (NUMMONS<<0)
	 */
	for (i = 1; artifact_names[i]; i++) continue;
	version.entity_count = (unsigned long)(i - 1);
	for (i = 1; objects[i].oc_class != ILLOBJ_CLASS; i++) continue;
	version.entity_count = (version.entity_count << 12) | (unsigned long)i;
	for (i = 0; mons[i].mlet; i++) continue;
	version.entity_count = (version.entity_count << 12) | (unsigned long)i;
	/*
	 * Value used for compiler (word size/field alignment/padding) check.
	 */
	version.struct_sizes = (((unsigned long)sizeof (struct flag)  << 24) |
				((unsigned long)sizeof (struct obj)   << 17) |
				((unsigned long)sizeof (struct monst) << 10) |
				((unsigned long)sizeof (struct you)));
	return;
}

static char *
version_string(outbuf)
char *outbuf;
{
    Sprintf(outbuf, "%d.%d.%d", VERSION_MAJOR, VERSION_MINOR, PATCHLEVEL);
#ifdef BETA
    Sprintf(eos(outbuf), "-%d", EDITLEVEL);
#endif
    return outbuf;
}

static char *
version_id_string(outbuf, build_date)
char *outbuf;
const char *build_date;
{
    char subbuf[64], versbuf[64];

    subbuf[0] = '\0';
#ifdef PORT_SUB_ID
    subbuf[0] = ' ';
    Strcpy(&subbuf[1], PORT_SUB_ID);
#endif
#ifdef BETA
    Strcat(subbuf, " Beta");
#endif

    Sprintf(outbuf, "%s NetHack%s Version %s - last build %s.",
	    PORT_ID, subbuf, version_string(versbuf), build_date);
    return outbuf;
}

void
do_date()
{
	long clocktim = 0;
	char *c, cbuf[60], buf[BUFSZ];
	const char *ul_sfx;

	filename[0]='\0';
#ifdef FILE_PREFIX
	Strcat(filename,file_prefix);
#endif
	Sprintf(eos(filename), INCLUDE_TEMPLATE, DATE_FILE);
	if (!(ofp = fopen(filename, WRTMODE))) {
		perror(filename);
		exit(EXIT_FAILURE);
	}
	Fprintf(ofp,"/*\tSCCS Id: @(#)date.h\t3.4\t2002/02/03 */\n\n");
	Fprintf(ofp,Dont_Edit_Code);

	(void) time((time_t *)&clocktim);
	Strcpy(cbuf, ctime((time_t *)&clocktim));
	for (c = cbuf; *c; c++) if (*c == '\n') break;
	*c = '\0';	/* strip off the '\n' */
	Fprintf(ofp,"#define BUILD_DATE \"%s\"\n", cbuf);
	Fprintf(ofp,"#define BUILD_TIME (%ldL)\n", clocktim);
	Fprintf(ofp,"\n");
	
	ul_sfx = "UL";
	Fprintf(ofp,"#define VERSION_NUMBER 0x%08lx%s\n",
		version.incarnation, ul_sfx);
	Fprintf(ofp,"#define VERSION_FEATURES 0x%08lx%s\n",
		version.feature_set, ul_sfx);
#ifdef IGNORED_FEATURES
	Fprintf(ofp,"#define IGNORED_FEATURES 0x%08lx%s\n",
		(unsigned long) IGNORED_FEATURES, ul_sfx);
#endif
	Fprintf(ofp,"#define VERSION_SANITY1 0x%08lx%s\n",
		version.entity_count, ul_sfx);
	Fprintf(ofp,"#define VERSION_SANITY2 0x%08lx%s\n",
		version.struct_sizes, ul_sfx);
	Fprintf(ofp,"\n");
	Fprintf(ofp,"#define VERSION_STRING \"%s\"\n", version_string(buf));
	Fprintf(ofp,"#define VERSION_ID \\\n \"%s\"\n",
		version_id_string(buf, cbuf));
	Fprintf(ofp,"\n");
	Fclose(ofp);
	return;
}

static char save_bones_compat_buf[BUFSZ];

static void
build_savebones_compat_string()
{
#ifdef VERSION_COMPATIBILITY
	unsigned long uver = VERSION_COMPATIBILITY;
#endif
	Strcpy(save_bones_compat_buf,
		"save and bones files accepted from version");
#ifdef VERSION_COMPATIBILITY
	Sprintf(eos(save_bones_compat_buf), "s %lu.%lu.%lu through %d.%d.%d",
		((uver & 0xFF000000L) >> 24), ((uver & 0x00FF0000L) >> 16),
		((uver & 0x0000FF00L) >> 8),
		VERSION_MAJOR, VERSION_MINOR, PATCHLEVEL);
#else
	Sprintf(eos(save_bones_compat_buf), " %d.%d.%d only",
		VERSION_MAJOR, VERSION_MINOR, PATCHLEVEL);
#endif
}

static const char *build_opts[] = {
#ifdef ANSI_DEFAULT
		"ANSI default terminal",
#endif
#ifdef AUTOPICKUP_EXCEPTIONS
		"autopickup_exceptions",
#endif
#ifdef TEXTCOLOR
		"color",
#endif
#ifdef COM_COMPL
		"command line completion",
#endif
#ifdef COMPRESS
		"data file compression",
#endif
#ifdef DLB
		"data librarian",
#endif
#ifdef WIZARD
		"debug mode",
#endif
#ifdef ELBERETH
		"Elbereth",
#endif
#ifdef EXP_ON_BOTL
		"experience points on status line",
#endif
#ifdef GOLDOBJ
		"gold object in inventories",
#endif
#ifdef INSURANCE
		"insurance files for recovering from crashes",
#endif
#ifdef KOPS
		"Keystone Kops",
#endif
#ifdef HOLD_LOCKFILE_OPEN
		"exclusive lock on level 0 file",
#endif
#ifdef LOGFILE
		"log file",
#endif
#ifdef NEWS
		"news file",
#endif
#ifdef OVERLAY
#  ifdef VROOMM
		"VROOMM overlays",
#  else
		"overlays",
#  endif
#endif
#ifdef REDO
		"redo command",
#endif
#ifdef REINCARNATION
		"rogue level",
#endif
#ifdef STEED
		"saddles and riding",
#endif
#ifdef SCORE_ON_BOTL
		"score on status line",
#endif
#ifdef CLIPPING
		"screen clipping",
#endif
#ifdef NO_TERMS
# ifdef SCREEN_VGA
		"screen control via VGA graphics",
# endif
# ifndef MSWIN_GRAPHICS
#  ifdef WIN32CON
		"screen control via WIN32 console I/O",
#  endif
# endif
#endif
#ifdef SEDUCE
		"seduction",
#endif
#ifdef SINKS
		"sinks",
#endif
#ifdef TERMINFO
		"terminal info library",
#else
# if defined(TERMLIB) || (!defined(WIN32) && defined(TTY_GRAPHICS))
		"terminal capability library",
# endif
#endif
#ifdef TIMED_DELAY
		"timed wait for display effects",
#endif
#ifdef TOURIST
		"tourists",
#endif
#ifdef USER_SOUNDS
# ifdef USER_SOUNDS_REGEX
		"user sounds via regular expressions",
# else
		"user sounds via pmatch",
# endif
#endif
#ifdef PREFIXES_IN_USE
		"variable playground",
#endif
#ifdef WALLIFIED_MAZE
		"walled mazes",
#endif
#ifdef ZEROCOMP
		"zero-compressed save files",
#endif
		save_bones_compat_buf,
		"basic NetHack features"
	};

static const char *window_opts[] = {
#ifdef TTY_GRAPHICS
		"traditional tty-based graphics",
#endif
#ifdef X11_GRAPHICS
		"X11",
#endif
#ifdef QT_GRAPHICS
		"Qt",
#endif
#ifdef GNOME_GRAPHICS
		"Gnome",
#endif
#ifdef GEM_GRAPHICS
		"Gem",
#endif
#ifdef MSWIN_GRAPHICS
		"mswin",
#endif
		0
	};

void
do_options()
{
	register int i, length;
	register const char *str, *indent = "    ";

	filename[0]='\0';
#ifdef FILE_PREFIX
	Strcat(filename,file_prefix);
#endif
	Sprintf(eos(filename), DATA_TEMPLATE, OPTIONS_FILE);
	if (!(ofp = fopen(filename, WRTMODE))) {
		perror(filename);
		exit(EXIT_FAILURE);
	}

	build_savebones_compat_string();
	Fprintf(ofp,
#ifdef BETA
		"\n    NetHack version %d.%d.%d [beta]\n",
#else
		"\n    NetHack version %d.%d.%d\n",
#endif
		VERSION_MAJOR, VERSION_MINOR, PATCHLEVEL);

	Fprintf(ofp,"\nOptions compiled into this edition:\n");

	length = COLNO + 1;	/* force 1st item onto new line */
	for (i = 0; i < SIZE(build_opts); i++) {
	    str = build_opts[i];
	    if (length + strlen(str) > COLNO - 5)
		Fprintf(ofp,"\n%s", indent),  length = strlen(indent);
	    else
		Fprintf(ofp," "),  length++;
	    Fprintf(ofp,"%s", str),  length += strlen(str);
	    Fprintf(ofp,(i < SIZE(build_opts) - 1) ? "," : "."),  length++;
	}

	Fprintf(ofp,"\n\nSupported windowing systems:\n");

	length = COLNO + 1;	/* force 1st item onto new line */
	for (i = 0; i < SIZE(window_opts) - 1; i++) {
	    str = window_opts[i];
	    if (length + strlen(str) > COLNO - 5)
		Fprintf(ofp,"\n%s", indent),  length = strlen(indent);
	    else
		Fprintf(ofp," "),  length++;
	    Fprintf(ofp,"%s", str),  length += strlen(str);
	    Fprintf(ofp, ","),  length++;
	}
	Fprintf(ofp, "\n%swith a default of %s.", indent, DEFAULT_WINDOW_SYS);
	Fprintf(ofp,"\n\n");

	Fclose(ofp);
	return;
}

/* routine to decide whether to discard something from data.base */
static boolean
d_filter(line)
    char *line;
{
    if (*line == '#') return TRUE;	/* ignore comment lines */
    return FALSE;
}

   /*
    *
	New format (v3.1) of 'data' file which allows much faster lookups [pr]
"do not edit"		first record is a comment line
01234567		hexadecimal formatted offset to text area
name-a			first name of interest
123,4			offset to name's text, and number of lines for it
name-b			next name of interest
name-c			multiple names which share same description also
456,7			share a single offset,count line
.			sentinel to mark end of names
789,0			dummy record containing offset, count of EOF
text-a			4 lines of descriptive text for name-a
text-a			at file position 0x01234567L + 123L
text-a
text-a
text-b/text-c		7 lines of text for names-b and -c
text-b/text-c		at fseek(0x01234567L + 456L)
...
    *
    */

void
do_data()
{
	char	infile[60], tempfile[60];
	boolean ok;
	long	txt_offset;
	int	entry_cnt, line_cnt;

	Sprintf(tempfile, DATA_TEMPLATE, "database.tmp");
	filename[0]='\0';
#ifdef FILE_PREFIX
	Strcat(filename,file_prefix);
#endif
	Sprintf(eos(filename), DATA_TEMPLATE, DATA_FILE);
	Sprintf(infile, DATA_IN_TEMPLATE, DATA_FILE);
	Strcat(infile,
#ifdef SHORT_FILENAMES
		".bas"
#else
		".base"
#endif
		);
	if (!(ifp = fopen(infile, RDTMODE))) {		/* data.base */
		perror(infile);
		exit(EXIT_FAILURE);
	}
	if (!(ofp = fopen(filename, WRTMODE))) {	/* data */
		perror(filename);
		Fclose(ifp);
		exit(EXIT_FAILURE);
	}
	if (!(tfp = fopen(tempfile, WRTMODE))) {	/* database.tmp */
		perror(tempfile);
		Fclose(ifp);
		Fclose(ofp);
		Unlink(filename);
		exit(EXIT_FAILURE);
	}

	/* output a dummy header record; we'll rewind and overwrite it later */
	Fprintf(ofp, "%s%08lx\n", Dont_Edit_Data, 0L);

	entry_cnt = line_cnt = 0;
	/* read through the input file and split it into two sections */
	while (fgets(in_line, sizeof in_line, ifp)) {
	    if (d_filter(in_line)) continue;
	    if (*in_line > ' ') {	/* got an entry name */
		/* first finish previous entry */
		if (line_cnt)  Fprintf(ofp, "%d\n", line_cnt),  line_cnt = 0;
		/* output the entry name */
		(void) fputs(in_line, ofp);
		entry_cnt++;		/* update number of entries */
	    } else if (entry_cnt) {	/* got some descriptive text */
		/* update previous entry with current text offset */
		if (!line_cnt)  Fprintf(ofp, "%ld,", ftell(tfp));
		/* save the text line in the scratch file */
		(void) fputs(in_line, tfp);
		line_cnt++;		/* update line counter */
	    }
	}
	/* output an end marker and then record the current position */
	if (line_cnt)  Fprintf(ofp, "%d\n", line_cnt);
	Fprintf(ofp, ".\n%ld,%d\n", ftell(tfp), 0);
	txt_offset = ftell(ofp);
	Fclose(ifp);		/* all done with original input file */

	/* reprocess the scratch file; 1st format an error msg, just in case */
	Sprintf(in_line, "rewind of \"%s\"", tempfile);
	if (rewind(tfp) != 0)  goto dead_data;
	/* copy all lines of text from the scratch file into the output file */
	while (fgets(in_line, sizeof in_line, tfp))
	    (void) fputs(in_line, ofp);

	/* finished with scratch file */
	Fclose(tfp);
	Unlink(tempfile);	/* remove it */

	/* update the first record of the output file; prepare error msg 1st */
	Sprintf(in_line, "rewind of \"%s\"", filename);
	ok = (rewind(ofp) == 0);
	if (ok) {
	   Sprintf(in_line, "header rewrite of \"%s\"", filename);
	   ok = (fprintf(ofp, "%s%08lx\n", Dont_Edit_Data, txt_offset) >= 0);
	}
	if (!ok) {
dead_data:  perror(in_line);	/* report the problem */
	    /* close and kill the aborted output file, then give up */
	    Fclose(ofp);
	    Unlink(filename);
	    exit(EXIT_FAILURE);
	}

	/* all done */
	Fclose(ofp);

	return;
}

/* routine to decide whether to discard something from oracles.txt */
static boolean
h_filter(line)
    char *line;
{
    static boolean skip = FALSE;
    char tag[sizeof in_line];

    if (*line == '#') return TRUE;	/* ignore comment lines */
    if (sscanf(line, "----- %s", tag) == 1) {
	skip = FALSE;
#ifndef SINKS
	if (!strcmp(tag, "SINKS")) skip = TRUE;
#endif
#ifndef ELBERETH
	if (!strcmp(tag, "ELBERETH")) skip = TRUE;
#endif
    } else if (skip && !strncmp(line, "-----", 5))
	skip = FALSE;
    return skip;
}

static const char *special_oracle[] = {
	"\"...it is rather disconcerting to be confronted with the",
	"following theorem from [Baker, Gill, and Solovay, 1975].",
	"",
	"Theorem 7.18  There exist recursive languages A and B such that",
	"  (1)  P(A) == NP(A), and",
	"  (2)  P(B) != NP(B)",
	"",
	"This provides impressive evidence that the techniques that are",
	"currently available will not suffice for proving that P != NP or          ",
	"that P == NP.\"  [Garey and Johnson, p. 185.]"
};

/*
   The oracle file consists of a "do not edit" comment, a decimal count N
   and set of N+1 hexadecimal fseek offsets, followed by N multiple-line
   records, separated by "---" lines.  The first oracle is a special case.
   The input data contains just those multi-line records, separated by
   "-----" lines.
 */

void
do_oracles()
{
	char	infile[60], tempfile[60];
	boolean in_oracle, ok;
	long	txt_offset, offset, fpos;
	int	oracle_cnt;
	register int i;

	Sprintf(tempfile, DATA_TEMPLATE, "oracles.tmp");
	filename[0]='\0';
#ifdef FILE_PREFIX
	Strcat(filename, file_prefix);
#endif
	Sprintf(eos(filename), DATA_TEMPLATE, ORACLE_FILE);
	Sprintf(infile, DATA_IN_TEMPLATE, ORACLE_FILE);
	Strcat(infile, ".txt");
	if (!(ifp = fopen(infile, RDTMODE))) {
		perror(infile);
		exit(EXIT_FAILURE);
	}
	if (!(ofp = fopen(filename, WRTMODE))) {
		perror(filename);
		Fclose(ifp);
		exit(EXIT_FAILURE);
	}
	if (!(tfp = fopen(tempfile, WRTMODE))) {	/* oracles.tmp */
		perror(tempfile);
		Fclose(ifp);
		Fclose(ofp);
		Unlink(filename);
		exit(EXIT_FAILURE);
	}

	/* output a dummy header record; we'll rewind and overwrite it later */
	Fprintf(ofp, "%s%5d\n", Dont_Edit_Data, 0);

	/* handle special oracle; it must come first */
	(void) fputs("---\n", tfp);
	Fprintf(ofp, "%05lx\n", ftell(tfp));  /* start pos of special oracle */
	for (i = 0; i < SIZE(special_oracle); i++) {
	    (void) fputs(xcrypt(special_oracle[i]), tfp);
	    (void) fputc('\n', tfp);
	}

	oracle_cnt = 1;
	(void) fputs("---\n", tfp);
	Fprintf(ofp, "%05lx\n", ftell(tfp));	/* start pos of first oracle */
	in_oracle = FALSE;

	while (fgets(in_line, sizeof in_line, ifp)) {

	    if (h_filter(in_line)) continue;
	    if (!strncmp(in_line, "-----", 5)) {
		if (!in_oracle) continue;
		in_oracle = FALSE;
		oracle_cnt++;
		(void) fputs("---\n", tfp);
		Fprintf(ofp, "%05lx\n", ftell(tfp));
		/* start pos of this oracle */
	    } else {
		in_oracle = TRUE;
		(void) fputs(xcrypt(in_line), tfp);
	    }
	}

	if (in_oracle) {	/* need to terminate last oracle */
	    oracle_cnt++;
	    (void) fputs("---\n", tfp);
	    Fprintf(ofp, "%05lx\n", ftell(tfp));	/* eof position */
	}

	/* record the current position */
	txt_offset = ftell(ofp);
	Fclose(ifp);		/* all done with original input file */

	/* reprocess the scratch file; 1st format an error msg, just in case */
	Sprintf(in_line, "rewind of \"%s\"", tempfile);
	if (rewind(tfp) != 0)  goto dead_data;
	/* copy all lines of text from the scratch file into the output file */
	while (fgets(in_line, sizeof in_line, tfp))
	    (void) fputs(in_line, ofp);

	/* finished with scratch file */
	Fclose(tfp);
	Unlink(tempfile);	/* remove it */

	/* update the first record of the output file; prepare error msg 1st */
	Sprintf(in_line, "rewind of \"%s\"", filename);
	ok = (rewind(ofp) == 0);
	if (ok) {
	    Sprintf(in_line, "header rewrite of \"%s\"", filename);
	    ok = (fprintf(ofp, "%s%5d\n", Dont_Edit_Data, oracle_cnt) >=0);
	}
	if (ok) {
	    Sprintf(in_line, "data rewrite of \"%s\"", filename);
	    for (i = 0; i <= oracle_cnt; i++) {
		if (!(ok = (fpos = ftell(ofp)) >= 0)) break;
		if (!(ok = (fseek(ofp, fpos, SEEK_SET) >= 0))) break;
		if (!(ok = (fscanf(ofp, "%5lx", &offset) == 1))) break;
		if (!(ok = (fseek(ofp, fpos, SEEK_SET) >= 0))) break;
		if (!(ok = (fprintf(ofp, "%05lx\n", offset + txt_offset) >= 0)))
		    break;
	    }
	}
	if (!ok) {
dead_data:  perror(in_line);	/* report the problem */
	    /* close and kill the aborted output file, then give up */
	    Fclose(ofp);
	    Unlink(filename);
	    exit(EXIT_FAILURE);
	}

	/* all done */
	Fclose(ofp);

	return;
}


static	struct deflist {

	const char	*defname;
	boolean	true_or_false;
} deflist[] = {
#ifdef REINCARNATION
	      {	"REINCARNATION", TRUE },
#else
	      {	"REINCARNATION", FALSE },
#endif
	      { 0, 0 } };

static int
check_control(s)
	char	*s;
{
	int	i;

	if(s[0] != '%') return(-1);

	for(i = 0; deflist[i].defname; i++)
	    if(!strncmp(deflist[i].defname, s+1, strlen(deflist[i].defname)))
		return(i);

	return(-1);
}

static char *
without_control(s)
	char *s;
{
	return(s + 1 + strlen(deflist[check_control(in_line)].defname));
}

void
do_dungeon()
{
	int rcnt = 0;

	Sprintf(filename, DATA_IN_TEMPLATE, DGN_I_FILE);
	if (!(ifp = fopen(filename, RDTMODE))) {
		perror(filename);
		exit(EXIT_FAILURE);
	}
	filename[0]='\0';
#ifdef FILE_PREFIX
	Strcat(filename, file_prefix);
#endif
	Sprintf(eos(filename), DGN_TEMPLATE, DGN_O_FILE);
	if (!(ofp = fopen(filename, WRTMODE))) {
		perror(filename);
		exit(EXIT_FAILURE);
	}
	Fprintf(ofp,Dont_Edit_Data);

	while (fgets(in_line, sizeof in_line, ifp) != 0) {
	    rcnt++;
	    if(in_line[0] == '#') continue;	/* discard comments */
recheck:
	    if(in_line[0] == '%') {
		int i = check_control(in_line);
		if(i >= 0) {
		    if(!deflist[i].true_or_false)  {
			while (fgets(in_line, sizeof in_line, ifp) != 0)
			    if(check_control(in_line) != i) goto recheck;
		    } else
			(void) fputs(without_control(in_line),ofp);
		} else {
		    Fprintf(stderr, "Unknown control option '%s' in file %s at line %d.\n",
			    in_line, DGN_I_FILE, rcnt);
		    exit(EXIT_FAILURE);
		}
	    } else
		(void) fputs(in_line,ofp);
	}
	Fclose(ifp);
	Fclose(ofp);

	return;
}

static boolean
ranged_attk(ptr)	/* returns TRUE if monster can attack at range */
	register struct permonst *ptr;
{
	register int	i, j;
	register int atk_mask = (1<<AT_BREA) | (1<<AT_SPIT) | (1<<AT_GAZE);

	for(i = 0; i < NATTK; i++) {
	    if((j=ptr->mattk[i].aatyp) >= AT_WEAP || (atk_mask & (1<<j)))
		return TRUE;
	}

	return(FALSE);
}

/* This routine is designed to return an integer value which represents
 * an approximation of monster strength.  It uses a similar method of
 * determination as "experience()" to arrive at the strength.
 */
static int
mstrength(ptr)
struct permonst *ptr;
{
	int	i, tmp2, n, tmp = ptr->mlevel;

	if(tmp > 49)		/* special fixed hp monster */
	    tmp = 2*(tmp - 6) / 4;

/*	For creation in groups */
	n = (!!(ptr->geno & G_SGROUP));
	n += (!!(ptr->geno & G_LGROUP)) << 1;

/*	For ranged attacks */
	if (ranged_attk(ptr)) n++;

/*	For higher ac values */
	n += (ptr->ac < 4);
	n += (ptr->ac < 0);

/*	For very fast monsters */
	n += (ptr->mmove >= 18);

/*	For each attack and "special" attack */
	for(i = 0; i < NATTK; i++) {

	    tmp2 = ptr->mattk[i].aatyp;
	    n += (tmp2 > 0);
	    n += (tmp2 == AT_MAGC);
	    n += (tmp2 == AT_WEAP && (ptr->mflags2 & M2_STRONG));
	}

/*	For each "special" damage type */
	for(i = 0; i < NATTK; i++) {

	    tmp2 = ptr->mattk[i].adtyp;
	    if ((tmp2 == AD_DRLI) || (tmp2 == AD_STON) || (tmp2 == AD_DRST)
		|| (tmp2 == AD_DRDX) || (tmp2 == AD_DRCO) || (tmp2 == AD_WERE))
			n += 2;
	    else if (strcmp(ptr->mname, "grid bug")) n += (tmp2 != AD_PHYS);
	    n += ((int) (ptr->mattk[i].damd * ptr->mattk[i].damn) > 23);
	}

/*	Leprechauns are special cases.  They have many hit dice so they
	can hit and are hard to kill, but they don't really do much damage. */
	if (!strcmp(ptr->mname, "leprechaun")) n -= 2;

/*	Finally, adjust the monster level  0 <= n <= 24 (approx.) */
	if(n == 0) tmp--;
	else if(n >= 6) tmp += ( n / 2 );
	else tmp += ( n / 3 + 1);

	return((tmp >= 0) ? tmp : 0);
}

void
do_monstr()
{
    register struct permonst *ptr;
    register int i, j;

    /*
     * create the source file, "monstr.c"
     */
    filename[0]='\0';
#ifdef FILE_PREFIX
    Strcat(filename, file_prefix);
#endif
    Sprintf(eos(filename), SOURCE_TEMPLATE, MON_STR_C);
    if (!(ofp = fopen(filename, WRTMODE))) {
	perror(filename);
	exit(EXIT_FAILURE);
    }
    Fprintf(ofp,Dont_Edit_Code);
    Fprintf(ofp,"#include \"config.h\"\n");
    Fprintf(ofp,"\nconst int monstr[] = {\n");
    for (ptr = &mons[0], j = 0; ptr->mlet; ptr++) {
	i = mstrength(ptr);
	Fprintf(ofp,"%2d,%c", i, (++j & 15) ? ' ' : '\n');
    }
    /* might want to insert a final 0 entry here instead of just newline */
    Fprintf(ofp,"%s};\n", (j & 15) ? "\n" : "");

    Fprintf(ofp,"\nvoid NDECL(monstr_init);\n");
    Fprintf(ofp,"\nvoid\n");
    Fprintf(ofp,"monstr_init()\n");
    Fprintf(ofp,"{\n");
    Fprintf(ofp,"    return;\n");
    Fprintf(ofp,"}\n");
    Fprintf(ofp,"\n/*monstr.c*/\n");

    Fclose(ofp);
    return;
}

void
do_permonst()
{
	int	i;
	char	*c, *nam;

	filename[0]='\0';
#ifdef FILE_PREFIX
	Strcat(filename, file_prefix);
#endif
	Sprintf(eos(filename), INCLUDE_TEMPLATE, MONST_FILE);
	if (!(ofp = fopen(filename, WRTMODE))) {
		perror(filename);
		exit(EXIT_FAILURE);
	}
	Fprintf(ofp,"/*\tSCCS Id: @(#)pm.h\t3.4\t2002/02/03 */\n\n");
	Fprintf(ofp,Dont_Edit_Code);
	Fprintf(ofp,"#ifndef PM_H\n#define PM_H\n");

	if (strcmp(mons[0].mname, "playermon") != 0)
		Fprintf(ofp,"\n#define\tPM_PLAYERMON\t(-1)");

	for (i = 0; mons[i].mlet; i++) {
		Fprintf(ofp,"\n#define\tPM_");
		if (mons[i].mlet == S_HUMAN &&
				!strncmp(mons[i].mname, "were", 4))
		    Fprintf(ofp, "HUMAN_");
		for (nam = c = tmpdup(mons[i].mname); *c; c++)
		    if (*c >= 'a' && *c <= 'z') *c -= (char)('a' - 'A');
		    else if (*c < 'A' || *c > 'Z') *c = '_';
		Fprintf(ofp,"%s\t%d", nam, i);
	}
	Fprintf(ofp,"\n\n#define\tNUMMONS\t%d\n", i);
	Fprintf(ofp,"\n#endif /* PM_H */\n");
	Fclose(ofp);
	return;
}


/*	Start of Quest text file processing. */
#include "qtext.h"

static struct qthdr	qt_hdr;
static struct msghdr	msg_hdr[N_HDR];
static struct qtmsg	*curr_msg;

static int	qt_line;

static boolean	in_msg;
#define NO_MSG	1	/* strlen of a null line returned by fgets() */

static boolean
qt_comment(s)
	char *s;
{
	if(s[0] == '#') return(TRUE);
	return((boolean)(!in_msg  && strlen(s) == NO_MSG));
}

static boolean
qt_control(s)
	char *s;
{
	return((boolean)(s[0] == '%' && (s[1] == 'C' || s[1] == 'E')));
}

static int
get_hdr (code)
	char *code;
{
	int	i;

	for(i = 0; i < qt_hdr.n_hdr; i++)
	    if(!strncmp(code, qt_hdr.id[i], LEN_HDR)) return (++i);

	return(0);
}

static boolean
new_id (code)
	char *code;
{
	if(qt_hdr.n_hdr >= N_HDR) {
	    Fprintf(stderr, OUT_OF_HEADERS, qt_line);
	    return(FALSE);
	}

	strncpy(&qt_hdr.id[qt_hdr.n_hdr][0], code, LEN_HDR);
	msg_hdr[qt_hdr.n_hdr].n_msg = 0;
	qt_hdr.offset[qt_hdr.n_hdr++] = 0L;
	return(TRUE);
}

static boolean
known_msg(num, id)
	int num, id;
{
	int i;

	for(i = 0; i < msg_hdr[num].n_msg; i++)
	    if(msg_hdr[num].qt_msg[i].msgnum == id) return(TRUE);

	return(FALSE);
}


static void
new_msg(s, num, id)
	char *s;
	int num, id;
{
	struct	qtmsg	*qt_msg;

	if(msg_hdr[num].n_msg >= N_MSG) {
		Fprintf(stderr, OUT_OF_MESSAGES, qt_line);
	} else {
		qt_msg = &(msg_hdr[num].qt_msg[msg_hdr[num].n_msg++]);
		qt_msg->msgnum = id;
		qt_msg->delivery = s[2];
		qt_msg->offset = qt_msg->size = 0L;

		curr_msg = qt_msg;
	}
}

static void
do_qt_control(s)
	char *s;
{
	char code[BUFSZ];
	int num, id = 0;

	switch(s[1]) {

	    case 'C':	if(in_msg) {
			    Fprintf(stderr, CREC_IN_MSG, qt_line);
			    break;
			} else {
			    in_msg = TRUE;
			    if (sscanf(&s[4], "%s %5d", code, &id) != 2) {
			    	Fprintf(stderr, UNREC_CREC, qt_line);
			    	break;
			    }
			    num = get_hdr(code);
			    if (!num && !new_id(code))
			    	break;
			    num = get_hdr(code)-1;
			    if(known_msg(num, id))
			    	Fprintf(stderr, DUP_MSG, qt_line);
			    else new_msg(s, num, id);
			}
			break;

	    case 'E':	if(!in_msg) {
			    Fprintf(stderr, END_NOT_IN_MSG, qt_line);
			    break;
			} else in_msg = FALSE;
			break;

	    default:	Fprintf(stderr, UNREC_CREC, qt_line);
			break;
	}
}

static void
do_qt_text(s)
	char *s;
{
	if (!in_msg) {
	    Fprintf(stderr, TEXT_NOT_IN_MSG, qt_line);
	}
	curr_msg->size += strlen(s);
	return;
}

static void
adjust_qt_hdrs()
{
	int	i, j;
	long count = 0L, hdr_offset = sizeof(int) +
			(sizeof(char)*LEN_HDR + sizeof(long)) * qt_hdr.n_hdr;

	for(i = 0; i < qt_hdr.n_hdr; i++) {
	    qt_hdr.offset[i] = hdr_offset;
	    hdr_offset += sizeof(int) + sizeof(struct qtmsg) * msg_hdr[i].n_msg;
	}

	for(i = 0; i < qt_hdr.n_hdr; i++)
	    for(j = 0; j < msg_hdr[i].n_msg; j++) {

		msg_hdr[i].qt_msg[j].offset = hdr_offset + count;
		count += msg_hdr[i].qt_msg[j].size;
	    }
	return;
}

static void
put_qt_hdrs()
{
	int	i;

	/*
	 *	The main header record.
	 */
#ifdef DEBUG
	Fprintf(stderr, "%ld: header info.\n", ftell(ofp));
#endif
	(void) fwrite((genericptr_t)&(qt_hdr.n_hdr), sizeof(int), 1, ofp);
	(void) fwrite((genericptr_t)&(qt_hdr.id[0][0]), sizeof(char)*LEN_HDR,
							qt_hdr.n_hdr, ofp);
	(void) fwrite((genericptr_t)&(qt_hdr.offset[0]), sizeof(long),
							qt_hdr.n_hdr, ofp);
#ifdef DEBUG
	for(i = 0; i < qt_hdr.n_hdr; i++)
		Fprintf(stderr, "%c @ %ld, ", qt_hdr.id[i], qt_hdr.offset[i]);

	Fprintf(stderr, "\n");
#endif

	/*
	 *	The individual class headers.
	 */
	for(i = 0; i < qt_hdr.n_hdr; i++) {

#ifdef DEBUG
	    Fprintf(stderr, "%ld: %c header info.\n", ftell(ofp),
		    qt_hdr.id[i]);
#endif
	    (void) fwrite((genericptr_t)&(msg_hdr[i].n_msg), sizeof(int),
							1, ofp);
	    (void) fwrite((genericptr_t)&(msg_hdr[i].qt_msg[0]),
			    sizeof(struct qtmsg), msg_hdr[i].n_msg, ofp);
#ifdef DEBUG
	    { int j;
	      for(j = 0; j < msg_hdr[i].n_msg; j++)
		Fprintf(stderr, "msg %d @ %ld (%ld)\n",
			msg_hdr[i].qt_msg[j].msgnum,
			msg_hdr[i].qt_msg[j].offset,
			msg_hdr[i].qt_msg[j].size);
	    }
#endif
	}
}

void
do_questtxt()
{
	Sprintf(filename, DATA_IN_TEMPLATE, QTXT_I_FILE);
	if(!(ifp = fopen(filename, RDTMODE))) {
		perror(filename);
		exit(EXIT_FAILURE);
	}

	filename[0]='\0';
#ifdef FILE_PREFIX
	Strcat(filename, file_prefix);
#endif
	Sprintf(eos(filename), DATA_TEMPLATE, QTXT_O_FILE);
	if(!(ofp = fopen(filename, WRBMODE))) {
		perror(filename);
		Fclose(ifp);
		exit(EXIT_FAILURE);
	}

	qt_hdr.n_hdr = 0;
	qt_line = 0;
	in_msg = FALSE;

	while (fgets(in_line, 80, ifp) != 0) {
	    qt_line++;
	    if(qt_control(in_line)) do_qt_control(in_line);
	    else if(qt_comment(in_line)) continue;
	    else		    do_qt_text(in_line);
	}

	(void) rewind(ifp);
	in_msg = FALSE;
	adjust_qt_hdrs();
	put_qt_hdrs();
	while (fgets(in_line, 80, ifp) != 0) {

		if(qt_control(in_line)) {
		    in_msg = (in_line[1] == 'C');
		    continue;
		} else if(qt_comment(in_line)) continue;
#ifdef DEBUG
		Fprintf(stderr, "%ld: %s", ftell(stdout), in_line);
#endif
		(void) fputs(xcrypt(in_line), ofp);
	}
	Fclose(ifp);
	Fclose(ofp);
	return;
}


static	char	temp[32];

static char *
limit(name,pref)	/* limit a name to 30 characters length */
char	*name;
int	pref;
{
	(void) strncpy(temp, name, pref ? 26 : 30);
	temp[pref ? 26 : 30] = 0;
	return temp;
}

void
do_objs()
{
	int i, sum = 0;
	char *c, *objnam;
	int nspell = 0;
	int prefix = 0;
	char class = '\0';
	boolean	sumerr = FALSE;

	filename[0]='\0';
#ifdef FILE_PREFIX
	Strcat(filename, file_prefix);
#endif
	Sprintf(eos(filename), INCLUDE_TEMPLATE, ONAME_FILE);
	if (!(ofp = fopen(filename, WRTMODE))) {
		perror(filename);
		exit(EXIT_FAILURE);
	}
	Fprintf(ofp,"/*\tSCCS Id: @(#)onames.h\t3.4\t2002/02/03 */\n\n");
	Fprintf(ofp,Dont_Edit_Code);
	Fprintf(ofp,"#ifndef ONAMES_H\n#define ONAMES_H\n\n");

	for(i = 0; !i || objects[i].oc_class != ILLOBJ_CLASS; i++) {
		objects[i].oc_name_idx = objects[i].oc_descr_idx = i;	/* init */
		if (!(objnam = tmpdup(OBJ_NAME(objects[i])))) continue;

		/* make sure probabilities add up to 1000 */
		if(objects[i].oc_class != class) {
			if (sum && sum != 1000) {
			    Fprintf(stderr, "prob error for class %d (%d%%)",
				    class, sum);
			    (void) fflush(stderr);
			    sumerr = TRUE;
			}
			class = objects[i].oc_class;
			sum = 0;
		}

		for (c = objnam; *c; c++)
		    if (*c >= 'a' && *c <= 'z') *c -= (char)('a' - 'A');
		    else if (*c < 'A' || *c > 'Z') *c = '_';

		switch (class) {
		    case WAND_CLASS:
			Fprintf(ofp,"#define\tWAN_"); prefix = 1; break;
		    case RING_CLASS:
			Fprintf(ofp,"#define\tRIN_"); prefix = 1; break;
		    case POTION_CLASS:
			Fprintf(ofp,"#define\tPOT_"); prefix = 1; break;
		    case SPBOOK_CLASS:
			Fprintf(ofp,"#define\tSPE_"); prefix = 1; nspell++; break;
		    case SCROLL_CLASS:
			Fprintf(ofp,"#define\tSCR_"); prefix = 1; break;
		    case AMULET_CLASS:
			/* avoid trouble with stupid C preprocessors */
			Fprintf(ofp,"#define\t");
			if(objects[i].oc_material == PLASTIC) {
			    Fprintf(ofp,"FAKE_AMULET_OF_YENDOR\t%d\n", i);
			    prefix = -1;
			    break;
			}
			break;
		    case GEM_CLASS:
			/* avoid trouble with stupid C preprocessors */
			if(objects[i].oc_material == GLASS) {
			    Fprintf(ofp,"/* #define\t%s\t%d */\n",
							objnam, i);
			    prefix = -1;
			    break;
			}
		    default:
			Fprintf(ofp,"#define\t");
		}
		if (prefix >= 0)
			Fprintf(ofp,"%s\t%d\n", limit(objnam, prefix), i);
		prefix = 0;

		sum += objects[i].oc_prob;
	}

	/* check last set of probabilities */
	if (sum && sum != 1000) {
	    Fprintf(stderr, "prob error for class %d (%d%%)", class, sum);
	    (void) fflush(stderr);
	    sumerr = TRUE;
	}

	Fprintf(ofp,"#define\tLAST_GEM\t(JADE)\n");
	Fprintf(ofp,"#define\tMAXSPELL\t%d\n", nspell+1);
	Fprintf(ofp,"#define\tNUM_OBJECTS\t%d\n", i);

	Fprintf(ofp, "\n/* Artifacts (unique objects) */\n\n");

	for (i = 1; artifact_names[i]; i++) {
		for (c = objnam = tmpdup(artifact_names[i]); *c; c++)
		    if (*c >= 'a' && *c <= 'z') *c -= (char)('a' - 'A');
		    else if (*c < 'A' || *c > 'Z') *c = '_';

		if (!strncmp(objnam, "THE_", 4))
			objnam += 4;
#ifdef TOURIST
		/* fudge _platinum_ YENDORIAN EXPRESS CARD */
		if (!strncmp(objnam, "PLATINUM_", 9))
			objnam += 9;
#endif
		Fprintf(ofp,"#define\tART_%s\t%d\n", limit(objnam, 1), i);
	}

	Fprintf(ofp, "#define\tNROFARTIFACTS\t%d\n", i-1);
	Fprintf(ofp,"\n#endif /* ONAMES_H */\n");
	Fclose(ofp);
	if (sumerr) exit(EXIT_FAILURE);
	return;
}

static char *
tmpdup(str)
const char *str;
{
	static char buf[128];

	if (!str) return (char *)0;
	(void)strncpy(buf, str, 127);
	return buf;
}

static char *
eos(str)
char *str;
{
    while (*str) str++;
    return str;
}


#ifdef STRICT_REF_DEF
NEARDATA struct flag flags;
# ifdef ATTRIB_H
struct attribs attrmax, attrmin;
# endif
#endif /* STRICT_REF_DEF */

/*makedefs.c*/
