/*
 *  CONFIG.C
 *
 *  Written on 30-Jul-90 by jim nutt.  Changes on 10-Jul-94 by John Dennis.
 *  Released to the public domain.
 *
 *  Finds & reads configuration files, initializes everything.
 */

#ifdef OS2
#define TEXTLEN    1024
#else
#define TEXTLEN    512
#endif
#define AREAS_BBS  0

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include "addr.h"
#include "areas.h"
#include "nedit.h"
#include "msged.h"
#include "winsys.h"
#include "menu.h"
#include "main.h"
#include "strextra.h"
#include "memextra.h"
#include "bmg.h"
#include "fecfg145.h"
#include "version.h"
#include "nshow.h"
#include "readmail.h"
#include "screen.h"
#include "config.h"
#include "environ.h"
#include "charset.h"

int areas_type;

static void applyflags(AREA *a, char *flagstring);

static char **tags2skip;
static char *mntdirunix = NULL, *mntdirdos = NULL, *areafileflags = NULL;
static GROUP *group = NULL;
static int num_groups = 0, check_area_files = 0;

static char *cfgverbs[] =
{
    "Name",
    "Address",
    "PrivateNet",
    "Alias",
    "OutFile",
    "Lastread",
    "TossLog",
    "UserList",
    "SwapPath",
    "NodePath",
    "Nodelist",
    "HelpFile",
    "AreaFile",
    "UserOffset",
    "Quote",
    "AlterFunc",
    "Switch",
    "Color",
    "Right",
    "QuoteRight",
    "TabSize",
    "CurStart",
    "CurEnd",
    "Fido",
    "Squish",
    "UUCP",
    "Domain",
    "Gate",
    "Origin",
    "ReadKey",
    "EditKey",
    "Function",
    "Include",
    "MaxX",
    "MaxY",
    "Template",
    "UucpName",
    "Group",
    "Colour",
    "Editor",
    "RobotName",
    "QuickBBS",
    "Quick",
    "Scan",
    "MountDir",
    "SoftCrXlat",
    "AreaExcl",
    "OutputCharset",
    "SortAreas",
    "EnableSC",
    "AreaFileFlags",
    "FreqArea",  /* now 51, should be 51 if possible */
    NULL
};

#define CFG_NAME           0
#define CFG_ADDRESS        1
#define CFG_PRIVATENET     2
#define CFG_ALIAS          3
#define CFG_OUTFILE        4
#define CFG_LASTREAD       5
#define CFG_TOSSLOG        6
#define CFG_USERLIST       7
#define CFG_SWAPPATH       8
#define CFG_NODEPATH       9
#define CFG_NODELIST       10
#define CFG_HELPFILE       11
#define CFG_AREAFILE       12
#define CFG_USEROFFSET     13
#define CFG_QUOTE          14
#define CFG_ALTERFUNC      15
#define CFG_SWITCH         16
#define CFG_COLOR          17
#define CFG_RIGHT          18
#define CFG_QUOTERIGHT     19
#define CFG_TABSIZE        20
#define CFG_CURSTART       21
#define CFG_CUREND         22
#define CFG_FIDO           23
#define CFG_SQUISH         24
#define CFG_UUCP           25
#define CFG_DOMAIN         26
#define CFG_GATE           27
#define CFG_ORIGIN         28
#define CFG_READKEY        29
#define CFG_EDITKEY        30
#define CFG_FUNCTION       31
#define CFG_INCLUDE        32
#define CFG_MAXX           33
#define CFG_MAXY           34
#define CFG_TEMPLATE       35
#define CFG_UUCPNAME       36
#define CFG_GROUP          37
#define CFG_COLOUR         38
#define CFG_EDITOR         39
#define CFG_ROBOTNAME      40
#define CFG_QUICKBBS       41
#define CFG_QUICK          42
#define CFG_SCAN           43
#define CFG_MOUNTDIR       44
#define CFG_SOFTCRXLAT     45
#define CFG_AREAEXCL       46
#define CFG_OUTPUTCHARSET  47
#define CFG_SORTAREAS      48
#define CFG_ENABLESC       49
#define CFG_AREAFILEFLAGS  50
#define CFG_FREQAREA       51

static struct colorverb colortable[] =
{
    {"Black", 0x00},
    {"Blue", 0x01},
    {"Green", 0x02},
    {"Cyan", 0x03},
    {"Red", 0x04},
    {"Magenta", 0x05},
    {"Brown", 0x06},
    {"LGrey", 0x07},
    {"DGrey", 0x08},
    {"LBlue", 0x09},
    {"LGreen", 0x0A},
    {"LCyan", 0x0B},
    {"LRed", 0x0C},
    {"LMagenta", 0x0D},
    {"Yellow", 0x0E},
    {"White", 0x0F},
    {"Intense", 0x08},
    {"_Black", 0x00},
    {"_Blue", 0x10},
    {"_Green", 0x20},
    {"_Cyan", 0x30},
    {"_Red", 0x40},
    {"_Magenta", 0x50},
    {"_Brown", 0x60},
    {"_LGrey", 0x70},
    {"_DGrey", 0x80},
    {"_LBlue", 0x90},
    {"_LGreen", 0xA0},
    {"_LCyan", 0xB0},
    {"_LRed", 0xC0},
    {"_LMagenta", 0xD0},
    {"_Yellow", 0xE0},
    {"_White", 0xF0},
    {"_Blink", 0x80},
    {NULL, 0}
};

static char *cfgcolors[] =
{
    "MainNorm",
    "MainQuote",
    "MainKludge",
    "MainTempl",
    "MainInfo",
    "MainDivide",
    "MainHeader",
    "MainHdrTxt",
    "MainBlock",
    "MainEdit",
    "MainWarn",
    "MainNetInf",
    "MenuBorder",
    "MenuNorm",
    "MenuSelect",
    "HelpBorder",
    "HelpTitle",
    "HelpNorm",
    "HelpHilight",
    "InfoBorder",
    "InfoNorm",
    "InputBorder",
    "InputNorm",
    "InputEdit",
    "DlgBorder",
    "DlgNorm",
    "DlgChNorm",
    "DlgChSel",
    "DlgEnNorm",
    "DlgEnSel",
    "DlgButSel",
    "DlgButNorm",
    "DlgButShadow",
    "ListBdr",
    "ListTitle",
    "ListNorm",
    "ListInfo",
    "ListSelect",
    NULL
};

static char *cfgswitches[] =
{
    "HardQuote",
    "SaveCC",
    "RawCC",
    "SEEN-BYs",
    "ShowNotes",
    "Confirm",
    "ShowAddr",
    "MSGIDs",
    "OpusDate",
    "DateArvd",
    "ChopQuote",
    "QQuotes",
    "UseLastr",
    "ShowCR",
    "ShowEOL",
    "RealMsgN",
    "UseMouse",
    "EditCROnly",
    "UsePID",
    "SOTEOT",
    "ShowTime",
    "ImportFN",
    "DMore",
    "StatBar",
    "ShowSystem",
    "ExtFormat",
    "AreaListExactMatch",
    "EchoFlags",
    "NetmailVia",
    "DomainOrigin",
    "RightNextUnreadArea",
    "ShowOrigins",
    "ShowTearLines",
    "TearLines",
    "OriginLines",
    "ShowSeenBys",
    "EditTearLines",
    "EditOriginLines",
    NULL
};

#define CFG_SW_HARDQUOTE            0
#define CFG_SW_SAVECC               1
#define CFG_SW_RAWCC                2
#define CFG_SW_SEENBYS              3
#define CFG_SW_SHOWNOTES            4
#define CFG_SW_CONFIRM              5
#define CFG_SW_SHOWADDR             6
#define CFG_SW_MSGIDS               7
#define CFG_SW_OPUSDATE             8
#define CFG_SW_DATEARVD             9
#define CFG_SW_CHOPQUOTE            10
#define CFG_SW_QQUOTES              11
#define CFG_SW_USELASTR             12
#define CFG_SW_SHOWCR               13
#define CFG_SW_SHOWEOL              14
#define CFG_SW_REALMSGN             15
#define CFG_SW_USEMOUSE             16
#define CFG_SW_EDITCRONLY           17
#define CFG_SW_USEPID               18
#define CFG_SW_SOTEOT               19
#define CFG_SW_SHOWTIME             20
#define CFG_SW_IMPORTFN             21
#define CFG_SW_DMORE                22
#define CFG_SW_STATBAR              23
#define CFG_SW_SHOWSYSTEM           24
#define CFG_SW_EXTFORMAT            25
#define CFG_SW_AREALISTEXACTMATCH   26
#define CFG_SW_ECHOFLAGS            27
#define CFG_SW_NETMAILVIA           28
#define CFG_SW_DOMAINORIGIN         29
#define CFG_SW_RIGHTNEXTUNREADAREA  30
#define CFG_SW_SHOWORIGINS          31
#define CFG_SW_SHOWTEARLINES        32
#define CFG_SW_TEARLINES            33
#define CFG_SW_ORIGINLINES          34
#define CFG_SW_SHOWSEENBYS          35
#define CFG_SW_EDITTEARLINES        36
#define CFG_SW_EDITORIGINLINES      37

char *pathcvt(char *path)
{
#ifdef UNIX
    int dospathlen, unixpathlen, i;
    if (mntdirdos != NULL)
    {
        dospathlen = strlen(mntdirdos);
    }
    else
    {
        dospathlen = 0;
    }
    if (mntdirunix != NULL)
    {
        unixpathlen = strlen(mntdirunix);
    }
    else
    {
        unixpathlen = 0;
    }
    if (mntdirdos != NULL && strncmp(path, mntdirdos, dospathlen) == 0)
    {
        char *temppath;
        temppath = xmalloc(unixpathlen + strlen(path) - dospathlen + 1);
        strcpy(temppath, mntdirunix);
        strcpy(temppath + unixpathlen, path + dospathlen);
        release(path);
        path = temppath;
    }
    for (i = strlen(path) - 1; i >= 0; i--)
    {
        if (path[i] == '\\')
        {
            path[i] = '/';
        }
    }
#endif
    return path;
}

static char wildcmp(char *s1, char *s2)
{
    while (*s1 != '\0' && *s2 != '\0')
    {
        if (tolower(*s1) == tolower(*s2) || *s1 == '?')
        {
            s1++;
            s2++;
        }
        else if (*s1 == '*')
        {
            while (*s1 && (*s1 == '*' || *s1 == '?'))
            {
                if (*s1 == '?')
                {
                    s2++;
                }
                s1++;
            }
            if (*s1 != '\0')
            {
                char *p;
                p = s1;
                while (*p != '\0' && *p != '*' && *p != '?')
                {
                    p++;
                }
                while (*s2 != '\0' && strnicmp(s2, s1, (size_t) (p - s1)))
                {
                    s2++;
                }
                if (*s2 != '\0')
                {
                    s2 += (size_t) (p - s1);
                    s1 = p;
                }
                else
                {
                    return 1;
                }
            }
            else
            {
                while (*s2 != '\0')
                {
                    s2++;
                }
            }
        }
        else
        {
            return 1;
        }
    }
    if (*s2 == '\0')
    {
        return 0;
    }
    else
    {
        return 1;
    }
}


/* Strips leading whitespaces from a string. */

char *striplwhite(char *str)
{
    while (*str && isspace((unsigned char)*str))
    {
        str++;
    }
    return str;
}


/* Strips trailing spaces from a string. */

char *striptwhite(char *str)
{
    char *p;

    if (str == NULL)
    {
        return str;
    }
    if (*str == 0)
    {
        return str;   /* strend is undefined for zero-length string! */
    }
    p = strend(str);
    while (p > str && *p && isspace((unsigned char)*p))
    {
        *p = '\0';
        p--;
    }
    return str;
}


/* Strips quotation marks ("Gaensefuesschen") from a string */

char *strip_geese_feet(char *str)
{
    char *p;

    if (str == NULL)
    {
        return str;
    }
    if (*str == 0)
    {
        return str;
    }
    p = str;
    while (*p && (isspace((unsigned char)*p) || *p == '\"') )
    {
       p++;
    }
    memmove(str, p, strlen(p) + 1);
    p = strend(str);
    while (p > str && *p && (isspace((unsigned char)*p) || *p == '\"'))
    {
        *p = '\0';
        p--;
    }
    return str;
}


/* Skips from current position to first blank found. */

char *skip_to_blank(char *str)
{
    if (str == NULL)
    {
        return str;
    }
    while (*str && !isspace((unsigned char)*str))
    {
        str++;
    }
    return str;
}


/* Kills a trailing slash from a path. */

void kill_trail_slash(char *str)
{
    char *p;

    if (str == NULL)
    {
        return;
    }
    if (*str == 0)
    {
        return;
    }
    p = strend(str);
    if (*p == '\\' || *p == '/')
    {
        *p = '\0';
    }
}


/*
 *  Returns the number of the passed verb in relation to the cfgverbs
 *  array, or -1 if the passed verb is not in the array.
 */

int GetVerbNum(char *verb)
{
    int i = 0;

    while (cfgverbs[i] != NULL)
    {
        if (stricmp(verb, cfgverbs[i]) == 0)
        {
            return i;
        }
        i++;
    }
    return -1;
}


/* Assigns the passed named area to the global colour array. */

void AssignColor(char *name, int color)
{
    int i = 0;

    while (cfgcolors[i] != NULL)
    {
        if (stricmp(name, cfgcolors[i]) == 0)
        {
            cm[i] = color;
            return;
        }
        i++;
    }
    printf("\nUnknown colour argument: '%s'\n", name);
}


/* Assigns the passed switch name to on or off. */

void AssignSwitch(char *swtch, int OnOff)
{
    int i = 0;

    while (cfgswitches[i] != NULL)
    {
        if (stricmp(swtch, cfgswitches[i]) == 0)
        {
            break;
        }
        i++;
    }

    if (cfgswitches[i] == NULL)
    {
        printf("\nUnknown switch: '%s'\n", swtch);
        return;
    }

    switch (i)
    {
    case CFG_SW_HARDQUOTE:
        SW->hardquote = OnOff;
        break;

    case CFG_SW_SAVECC:
        SW->savecc = OnOff;
        break;

    case CFG_SW_RAWCC:
        SW->rawcc = OnOff;
        break;

    case CFG_SW_SEENBYS:
    case CFG_SW_SHOWSEENBYS:
        SW->showseenbys = OnOff;
        break;

    case CFG_SW_SHOWORIGINS:
        SW->showorigins = OnOff;
        break;

    case CFG_SW_SHOWTEARLINES:
        SW->showtearlines = OnOff;
        break;

    case CFG_SW_SHOWNOTES:
        SW->shownotes = OnOff;
        break;

    case CFG_SW_CONFIRM:
        SW->confirmations = OnOff;
        break;

    case CFG_SW_SHOWADDR:
        SW->showaddr = OnOff;
        break;

    case CFG_SW_MSGIDS:
        SW->msgids = OnOff;
        break;

    case CFG_SW_OPUSDATE:
        SW->opusdate = OnOff;
        break;

    case CFG_SW_DATEARVD:
        SW->datearrived = OnOff;
        break;

    case CFG_SW_CHOPQUOTE:
        SW->chopquote = OnOff;
        break;

    case CFG_SW_QQUOTES:
        SW->qquote = OnOff;
        break;

    case CFG_SW_USELASTR:
        SW->use_lastr = OnOff;
        break;

    case CFG_SW_SHOWCR:
        SW->showcr = OnOff;
        break;

    case CFG_SW_SHOWEOL:
        SW->showeol = OnOff;
        break;

    case CFG_SW_REALMSGN:
        SW->showrealmsgn = OnOff;
        break;

    case CFG_SW_USEMOUSE:
        SW->usemouse = OnOff;
        break;

    case CFG_SW_EDITCRONLY:
        SW->editcronly = OnOff;
        break;

    case CFG_SW_USEPID:
        SW->usepid = OnOff;
        break;

    case CFG_SW_SOTEOT:
        SW->soteot = OnOff;
        break;

    case CFG_SW_SHOWTIME:
        SW->showtime = OnOff;
        break;

    case CFG_SW_IMPORTFN:
        SW->importfn = OnOff;
        break;

    case CFG_SW_DMORE:
        SW->dmore = OnOff;
        break;

    case CFG_SW_STATBAR:
        SW->statbar = OnOff;
        break;

    case CFG_SW_SHOWSYSTEM:
        SW->showsystem = OnOff;
        break;

    case CFG_SW_EXTFORMAT:
        SW->extformat = OnOff;
        break;

    case CFG_SW_AREALISTEXACTMATCH:
        SW->arealistexactmatch = OnOff;
        break;

    case CFG_SW_ECHOFLAGS:
        SW->echoflags = OnOff;
        break;

    case CFG_SW_NETMAILVIA:
        SW->netmailvia = OnOff;
        break;

    case CFG_SW_DOMAINORIGIN:
        SW->domainorigin = OnOff;
        break;

    case CFG_SW_RIGHTNEXTUNREADAREA:
        SW->rightnextunreadarea = OnOff;
        break;

    case CFG_SW_TEARLINES:
        SW->usetearlines = OnOff;
        break;

    case CFG_SW_ORIGINLINES:
        SW->useoriginlines = OnOff;
        break;

    case CFG_SW_EDITTEARLINES:
        SW->edittearlines = OnOff;
        break;

    case CFG_SW_EDITORIGINLINES:
        SW->editoriginlines = OnOff;
        break;

    default:
        printf("\nUnknown switch: '%s'\n", swtch);
        break;
    }
}


/*
 *  Searches through the colortable for the matching colorname and
 *  returns the color, or -1 if not found.
 */

int GetColor(char *color)
{
    int i = 0;

    while (colortable[i].name != NULL)
    {
        if (stricmp(color, colortable[i].name) == 0)
            return colortable[i].color;
        i++;
    }
    printf("\nUnknown color: '%s'\n", color);
    return -1;
}


/*
 *  Determines if the passed char is a token seperator.
 */

int toksep(char ch)
{
    return strchr(" ,\r\t\n", ch) != NULL ? TRUE : FALSE;
}


/*
 *  Gets num tokens from a string, seperated by a default set of tokens.
 *  Handles stuff bewtween quotes as one token.
 */

void parse_tokens(char *str, char *tokens[], int num)
{
    int i = 0;
    char *s = str;
    int done = 0;

    while (!done && i < num)
    {
        if (toksep(*s))
        {
            while (*s && toksep(*s))
            {
                s++;
            }
        }

        if (*s == ';')
        {
            tokens[i] = NULL;
            break;
        }

        if (*s == '\"')
        {
            tokens[i++] = ++s;

            while (*s && *s != '\"')
            {
                s++;
            }
            if (*s == '\"')
            {
                *s++ = '\0';
            }
        }
        else
        {
            tokens[i++] = s;
            while (*s && !toksep(*s))
            {
                s++;
            }
        }
        if (*s == '\0')
        {
            tokens[i] = NULL;
            done = TRUE;
        }
        else
        {
            *s++ = '\0';
        }
    }
    tokens[i] = NULL;
}

/*
 *  Opens cfn first, and returns if successful, else it tries env.
 *  env can be considered the default name.
 */

static FILE *fileopen(char *env, char *cfn)
{
    FILE *fp;
    if (cfn != NULL)
    {
        fp = fopen(cfn, "r");
        if (fp != NULL)
        {
            return fp;
        }
    }
    fp = fopen(env, "r");
    return fp;
}

/*
 *  Parses a macro.
 */

static unsigned int *parse_macro(char *macro)
{
    unsigned int *m, *t;
    int l;
    char tmp[5];

    t = xcalloc(strlen(macro) * 2, sizeof *t);
    m = t;

    if (t == NULL || macro == NULL)
    {
        return NULL;
    }

    while (*macro)
    {
        if (*macro == '^')
        {
            *t++ = (unsigned int)(*(macro + 1) == '^') ? '^' : toupper(*(macro + 1) - 64);
            macro += 2;
        }
        else if (*macro == '\\')
        {
            if (*(macro + 1) == '\\')
            {
                *t++ = (unsigned int)'\\';
                macro += 2;
            }
            else
            {
                memset(tmp, 0, sizeof tmp);
                strncpy(tmp, macro + 1, 4);
                *t++ = (unsigned int)strtol(tmp, NULL, 0) << 8;
                macro += 5;
            }
        }
        else
        {
            *t++ = (unsigned int)*macro++;
        }
    }

    *t = 0;
    l = (int)(t - m) + 1;

    t = xrealloc(m, l * sizeof(int));

    if (t != NULL)
    {
        return t;
    }

    return m;
}

static void SkipArea(char * a)
{
    int i;
    if (tags2skip == NULL)
    {
         tags2skip = xmalloc(sizeof *tags2skip);
         tags2skip[0] = NULL;
    }
    i = 0;
    while (tags2skip[i] != NULL && stricmp(tags2skip[i], a) != 0)
    {
        i++;
    }
    if (tags2skip[i] == NULL)
    {
        tags2skip = xrealloc(tags2skip, sizeof *tags2skip * (i + 2));
        tags2skip[i] = xstrdup(a);
        tags2skip[i + 1] = NULL;
    }
}

/*
 *  Sets the username and the template for an area.
 */

static void SetAreaInfo(AREA * a)
{
    int i;

    for (i = 0; i < num_groups; i++)
    {
        if (bmg_find(a->description, group[i].search) != NULL)
        {
            break;
        }
    }

    if (i != num_groups)
    {
        a->username = group[i].username;
        a->template = group[i].template;
    }
}

/*
 *  Adds a new area to the area-list.
 */

static void AddArea(AREA * a)
{
    int i, g;
    char *d, *t, *p;

    if (tags2skip != NULL)
    {
        i = 0;
        while (tags2skip[i] != NULL && wildcmp(tags2skip[i], a->tag) != 0)
        {
            i++;
        }
        if (tags2skip[i] != NULL)
        {
            return;
        }
    }

    if (a->msgtype == QUICK && ST->quickbbs == NULL)
    {
        printf("\nFor QuickBBS areas, set QuickBBS path!\n");
        return;
    }

    for (i = 0; i < SW->areas; i++)
    {
        if ((arealist[i].path != NULL) && (((a->msgtype == QUICK) &&
          (arealist[i].msgtype == QUICK) && (a->board == a->board)) ||
          ((a->msgtype != QUICK) && (stricmp(a->path, arealist[i].path) == 0))))
        {
            break;
        }
    }

    if (i == SW->areas)
    {
        /* It's a new area, so we add it to the end of the list. */

        SW->areas++;
        SW->area = SW->areas - 1;
        if (SW->areas > SW->maxareas)
        {
            SW->maxareas += 50;
                /* Reallocating in greater blocks dramatically reduces */
                /* memory requirements of the DOS version. At least    */
                /* Borlands realloc does not seem to be very "intelligent".. */

            arealist = xrealloc(arealist, SW->maxareas * sizeof(struct _area));
        }

        memset(&CurArea, 0, sizeof(struct _area));

        CurArea = *a;

        if (a->addr.domain)
        {
            CurArea.addr.domain = xstrdup(a->addr.domain);
        }

        CurArea.description = xstrdup(a->description);
        CurArea.tag = xstrdup(a->tag);
        if (a->msgtype != QUICK)
        {
            CurArea.path = xstrdup(a->path);
            kill_trail_slash(CurArea.path);
            CurArea.path = pathcvt(CurArea.path);
        }
    }
    else
    {
        /*
         *  We redefine the area to the new defaults, with the
         *  exception of the path, tag, desc and group. */

        release(arealist[i].addr.domain);
        p = arealist[i].path;
        t = arealist[i].tag;
        d = arealist[i].description;
        g = arealist[i].group;

        arealist[i] = *a;
        arealist[i].path = p;
        arealist[i].tag = t;
        arealist[i].description = d;
        arealist[i].group = g;

        if (a->addr.domain)
        {
            arealist[i].addr.domain = xstrdup(a->addr.domain);
        }
    }

    SetAreaInfo(&arealist[i]);

    release(a->tag);
    release(a->description);
    release(a->path);
    release(a->addr.domain);
}

/*
 *  Checks a file in the areas.bbs format and gets all the area
 *  definitions. First attempts to open the file specified on the
 *  command line, and if that fails, tries for the default name
 *  "areas.bbs".
 */

static void checkareas(char *areafile)
{
    static AREA a;                /* current area */
    static char buffer[TEXTLEN];  /* line buffer */
    FILE *fp;                     /* file handle */
    char *s;                      /* pointer */
    int flag = 1;                 /* found host name? */
    char *tokens[12];

#ifdef USE_MSGAPI
    int sq = 0;                   /* a squish base? */
#endif

    fp = fileopen("areas.bbs", areafile);
    if (fp == NULL)
    {
        return;
    }

    while (fgets(buffer, TEXTLEN, fp) != NULL)
    {
        if (*buffer == '\r' || *buffer == '\n')
        {
            continue;
        }

        s = striplwhite(buffer);

        if (*s == ';' || *s == '-' || *s == '#')
        {
            continue;
        }

        if (strchr(buffer, '!') && flag)
        {
            char *p;
            p = strrchr(buffer, '!');
            if (p != NULL)
            {
                *p = '\0';
                release(ST->origin);
                ST->origin = xstrdup(buffer);
            }
            flag = 0;
            continue;
        }

#ifdef USE_MSGAPI
        if (*s == '$')
        {
            sq = 1;
            s++;
        }
        else
        {
            sq = 0;
        }
#endif

        memset(tokens, 0, sizeof tokens);
        parse_tokens(s, tokens, 4);

        if (tokens[0] == NULL || tokens[1] == NULL)
        {
            continue;
        }

        memset(&a, 0, sizeof a);

        a.echomail = 1;
        a.msgtype = FIDO;

        sscanf(tokens[0], " %d ", &a.board);
        if (a.board)
        {
            a.msgtype = QUICK;
        }
        else
        {
            a.path = xstrdup(tokens[0]);
        }

        a.tag = xstrdup(tokens[1]);
        a.description = xstrdup(tokens[1]);
        a.addr.notfound = 1;

        kill_trail_slash(a.path);
        a.path = pathcvt(a.path);

        if (tokens[2] != NULL && strlen(tokens[2]) >= 7)
        {
            s = tokens[2];
            if (*s == '-' && toupper(*(s + 1)) == 'P')
            {
                a.addr = parsenode(s + 2);
            }
        }

        if (a.addr.notfound)
        {
            a.addr = thisnode;
            if (thisnode.domain)
            {
                a.addr.domain = xstrdup(thisnode.domain);
            }
        }

        strupr(a.tag);

#ifdef USE_MSGAPI
        if (sq)
        {
            a.msgtype = SQUISH;
        }
#endif

        applyflags(&a, areafileflags);
        AddArea(&a);
    }

    fclose(fp);
}

/* check_fastecho - reads the areas in a FastEcho 1.45 configuration file */

static void check_fastecho(char *areafile)
{
    static AREA a;              /* current area */
    FILE *fp;                   /* file handle */
    LOOKUPTABLE *ltable;        /* for description charset conversion */
#ifdef USE_MSGAPI
    int sq = 0;                 /* a squish base? */
#endif
    CONFIG feconfig;
    Area fearea;
    ExtensionHeader feexthdr;
    SysAddress feakas[MAX_AKAS];
    int i, cnt, found;
    unsigned char atype, storage;
    static char progress_indicators[4] =
    {'-', '\\', '|', '/'};
    dword curofs;
    char *tempdsc;

    if (alias == NULL)
    {
        printf("\nError! Primary address must be defined.\n");
        exit(-1);
    }

    if (user_list[0].name == NULL)
    {
        printf("\nError! Name must be defined.\n");
        exit(-1);
    }

    fp = fopen(areafile, "rb");
    if (fp == NULL)
    {
        return;
    }

    ST->fecfgpath = xstrdup(areafile);

    if (fread(&feconfig, sizeof feconfig, 1, fp) != 1)
    {
        return;
    }

    /* preload the AKAs */

    fseek(fp, sizeof feconfig, SEEK_SET);
    curofs = (dword) sizeof feconfig; found=0;

    while (curofs < sizeof feconfig + feconfig.offset)
    {
        if (fread(&feexthdr, sizeof feexthdr, 1, fp) != 1)
        {
            return;
        }
        curofs += sizeof feexthdr;
        if (feexthdr.type == EH_AKAS)
        {
          cnt = 0; found = 1;
          do
            {
               if (fread(feakas + cnt, sizeof feakas[0], 1, fp) != 1)
               {
                   return;
               }
               curofs += sizeof feakas[0]; cnt++;
           }
           while (cnt < feconfig.AkaCnt);
        }
        else
        {
           fseek(fp, feexthdr.offset, SEEK_CUR);
           curofs += feexthdr.offset;
        }

    }
    if (!found)  /* no EH_AKAS header !? */
    {
        return;
    }

    ltable = get_readtable("IBMPC", 2);
                                /* we assume that umlauts in the
                                   fastecho.cfg area descriptions are
                                   always in the IBMPC charset. */

    /* set up the fastecho primary netmail path */
    memset(&a, 0, sizeof a);
    a.msgtype = FIDO;
    a.group = -1;
    a.echomail = 0;
    a.netmail = 1; a.priv = 1;
    a.addr.notfound = 0;
    a.addr.fidonet = 1;
    a.addr.zone = feakas[0].main.zone;
    a.addr.net = feakas[0].main.net;
    a.addr.node = feakas[0].main.node;
    a.addr.point = feakas[0].main.point;
    if (feakas[0].domain)
    {
        if (*feakas[0].domain)
        {
            a.addr.domain = xstrdup(feakas[0].domain);
        }
    }
    a.path = env_expand(feconfig.NetMPath);
    a.tag = xstrdup("NETMAIL");
    a.description = xstrdup("NETMAIL - FastEcho Primary Netmail Folder");
    kill_trail_slash(a.path);
    strlwr(a.path);
    applyflags (&a, areafileflags);
    AddArea(&a);

    /* Scan for echomail and secondary netmail areas */

    fseek(fp, (dword)(sizeof feconfig) + feconfig.offset + feconfig.NodeCnt *
      feconfig.NodeRecSize, SEEK_SET);

    for (i = 0; i < feconfig.AreaCnt; i++)
    {
        /* printing an indicator after EVERY area slows down the system
           unnecessary when running in an OS/2 or Windows windowed session,
           now that reading a single area is done in almost zero time */
        if (i%50)
        {
            printf("%c\b", progress_indicators[(i / 50) % 4]);
            fflush(stdout);
        }

        if(fread(&fearea, sizeof fearea, 1, fp) != 1)
        {
            return;
        }

        cnt = 0;
        found = 0;

        /* AKA index is  (fearea.info.akagroup & 0xff) */

        /* set things up */

        memset(&a, 0, sizeof a);
        a.msgtype = FIDO;
        a.group = (fearea.info.akagroup & 0xff00) >> 8;
        a.echomail = 1;
        a.netmail = 0;
        a.addr.notfound = 0;
        a.addr.fidonet = 1;
        a.addr.zone = feakas[fearea.info.akagroup & 0xff].main.zone;
        a.addr.net = feakas[fearea.info.akagroup & 0xff].main.net;
        a.addr.node = feakas[fearea.info.akagroup & 0xff].main.node;
        a.addr.point = feakas[fearea.info.akagroup & 0xff].main.point;
        if (feakas[fearea.info.akagroup & 0xff].domain)
        {
            if (*feakas[fearea.info.akagroup & 0xff].domain)
            {
                a.addr.domain =
                  xstrdup(feakas[fearea.info.akagroup & 0xff].domain);
            }
        }

        /* storage type: QBBS, etc... */
        storage = fearea.flags.flags & 0x0f;
        atype = (fearea.flags.flags & 0xf0) >> 4;

        if (atype == AREA_NETMAIL)
        {
            a.echomail = 0;
            a.netmail = 1;
            a.priv = 1;
        }
        else
        if (atype == AREA_LOCAL)
        {
            a.echomail = 0;
            a.local = 1;
            a.priv = 0;
        }

#ifdef USE_MSGAPI
        if (storage == FE_SQUISH)
        {
            sq = 1;
        }
        else
        {
            sq = 0;
        }
#endif

        if (storage == FE_QBBS)
        {
            a.msgtype = QUICK;
        }
        else if (storage == FE_JAM || storage == FE_PASSTHRU)
        {
            continue;  /* JAM is unsupported */
        }

        if (a.msgtype == QUICK)
        {
            a.board = fearea.board;
        }
        else
        {
            a.path = env_expand(fearea.path);
        }

        a.tag = xstrdup(fearea.name);
        strupr(a.tag);

        if (fearea.desc && *fearea.desc)
        {
            tempdsc = translate_text(fearea.desc, ltable);
            a.description =
              xmalloc(strlen(fearea.name) + strlen(tempdsc) + 4 );

            sprintf (a.description, "%s - %s", fearea.name, tempdsc);
            release(tempdsc);
        }
        else
        {
            a.description = xstrdup(fearea.name);
        }

        if (a.path)
        {
            kill_trail_slash(a.path);
            a.path = pathcvt(a.path);
        }

#ifdef USE_MSGAPI
        if (sq)
        {
            a.msgtype = SQUISH;
        }
#endif
        applyflags(&a, areafileflags);
        AddArea(&a);
    }

    fclose(fp);
}

/* check_squish - reads the areas in a squish configuration file */

static void check_squish(char *areafile)
{
    static AREA a;
    static char progress_indicators[4] =
    {'-', '\\', '|', '/'};
    static char raw_buffer[TEXTLEN];
    char *buffer = NULL;
    char *tokens[20];
    FILE *fp;
    int i, line_num = 0;
    char *s;

    if (alias == NULL)
    {
        printf("\nError! Primary address must be defined.\n");
        exit(-1);
    }

    if (user_list[0].name == NULL)
    {
        printf("\nError! Name must be defined.\n");
        exit(-1);
    }

    fp = fileopen("squish.cfg", areafile);
    if (fp == NULL)
    {
        return;
    }

    memset(raw_buffer, 0, TEXTLEN);

    while (fgets(raw_buffer, TEXTLEN, fp) != NULL)
    {
        line_num++;
        release(buffer);
        buffer = env_expand(raw_buffer);

        printf("%c\b", progress_indicators[line_num % 2]);
        fflush(stdout);

        if (*buffer == '\r' || *buffer == '\n')
        {
            continue;
        }

        s = striplwhite(buffer);

        if (*s == ';' || *s == '-' || *s == '#')
        {
            continue;
        }

        memset(tokens, 0, sizeof(tokens));
        parse_tokens(s, tokens, 15);

        if (tokens[0] == NULL)
        {
            continue;
        }

        if (!stricmp(tokens[0], "EchoArea") ||
          !stricmp(tokens[0], "LocaLarea") ||
          !stricmp(tokens[0], "NetArea") ||
          !stricmp(tokens[0], "BadArea") ||
          !stricmp(tokens[0], "DupeArea"))
        {
            if (tokens[1] == NULL || tokens[2] == NULL)
            {
                continue;
            }

            memset(&a, 0, sizeof a);

            if (!stricmp(tokens[0], "EchoArea"))
            {
                a.echomail = 1;
            }
            else if (!stricmp(tokens[0], "LocalArea"))
            {
                a.local = 1;
            }
            else
            {
                a.netmail = 1;
                a.priv = 1;
            }

            a.msgtype = FIDO;
            a.addr.notfound = 1;
            a.tag = xstrdup(tokens[1]);
            a.path = pathcvt(xstrdup(tokens[2]));
            a.description = xstrdup(tokens[1]);

            strupr(a.tag);

            i = 3;
            while ((s = tokens[i]) != NULL)
            {
                if (*s == ';')
                {
                    i = 0;
                    break;
                }
#ifdef USE_MSGAPI
                else if (*s == '-' && *(s + 1) && *(s + 1) == '$')
                {
                    a.msgtype = SQUISH;
                }
#endif
                else if (*s == '-' && *(s + 1) && toupper(*(s + 1)) == 'P')
                {
                    /* an address was specified */

                    release(a.addr.domain);
                    a.addr = parsenode(s + 2);
                }
                else if (*s == '-' && *(s + 1) && *(s + 1) == '0')
                {
                    /* ignore passthru area */

                    release(a.tag);
                    release(a.description);
                    release(a.path);
                    release(a.addr.domain);
                    i = 0;
                    break;
                }
                i++;
            }

            if (i == 0)
            {
                continue;
            }

            if (a.addr.notfound)
            {
                a.addr = thisnode;
                if (thisnode.domain)
                {
                    a.addr.domain = xstrdup(thisnode.domain);
                }
            }
            applyflags(&a, areafileflags);
            AddArea(&a);
        }
    }

    release(buffer);
    fclose(fp);
}


/*
 *  Applies a string of user-given flags to an area definition
 */

static void applyflags(AREA *a, char *flagstring)
{
    char *s = flagstring;

    if (s == NULL)
    {
        return;
    }

    while (*s)
    {
        switch (tolower(*s))  /* one letter is enough */
        {
        case 'n':
            a->news = 1;
            break;
        case 'u':
            a->uucp = 1;
            break;
        case 'm':
            a->netmail = 1;
            break;
        case 'e':
            a->echomail = 1;
            break;
        case 'p':
            a->priv = 1;
            break;
        case 'h':
            a->hold = 1;
            break;
        case 'd':
            a->direct = 1;
            break;
        case 'c':
            a->crash = 1;
            break;
        case 'k':
            a->killsent = 1;
            break;
        case 'l':
            a->local = 1;
            break;
        case '8':
            a->eightbits = 1;
            break;
        default:
            /* unknown */
            break;
        }
        s++;
    }

    if (!a->echomail && !a->netmail && !a->uucp && !a->news)
    {
        a->local = 1;
    }
}


/*
 *  Reads in an area definition in the msged.cfg format.
 */

static void parsemail(char *keyword, char *value)
{
    char *tokens[20];
    static AREA a;

    memset(&a, 0, sizeof a);
    memset(tokens, 0, sizeof tokens);

    if (alias == NULL)
    {
        printf("\nError! Primary address must be defined.\n");
        exit(-1);
    }

    if (user_list[0].name == NULL)
    {
        printf("\nError! Name must be defined.\n");
        exit(-1);
    }

    switch (tolower(*keyword))  /* one letter is enough for now */
    {
    default:
    case 'f':
        a.msgtype = FIDO;
        break;
    case 'q':
        a.msgtype = QUICK;
        break;
#ifdef USE_MSGAPI
    case 's':
        a.msgtype = SQUISH;
        break;
#endif
    }

    parse_tokens(value, tokens, 5);

    if (tokens[2] == NULL)
    {
        return;
    }

    applyflags(&a, tokens[0]);

    a.description = xstrdup(tokens[1]);
    switch (a.msgtype)
    {
    case QUICK:
        a.board = atoi(tokens[2]);
        break;
    default:
        a.path = pathcvt(xstrdup(tokens[2]));
        break;
    }

    a.addr.notfound = 1;

    if (a.echomail)
    {
        if (tokens[3] != NULL)
        {
            a.tag = xstrdup(tokens[3]);
            if (tokens[4] != NULL)
            {
                a.addr = parsenode(tokens[4]);
            }
        }
        else
        {
            a.tag = xstrdup(a.description);
        }
    }
    else
    {
        a.tag = xstrdup(a.description);
        if (tokens[3] != NULL)
        {
            a.addr = parsenode(tokens[3]);
        }
    }

    if (a.addr.notfound)
    {
        a.addr = thisnode;
        if (thisnode.domain)
        {
            a.addr.domain = xstrdup(thisnode.domain);
        }
    }
    AddArea(&a);
}

/*
 *  Parses an alias.
 */

static void parse_alias(char *value)
{
    static ALIAS a;
    char *tokens[6];
    char *s;

    memset(&a, 0, sizeof a);
    memset(tokens, 0, sizeof tokens);

    parse_tokens(value, tokens, 5);

    if (tokens[2] == NULL)
    {
        return;
    }

    a.alias = xstrdup(tokens[0]);
    a.name = xstrdup(tokens[1]);

    if (!stricmp(tokens[2], "uucp"))
    {
     /*   a.addr.internet = 1; */
    }
    else
    {
        a.addr = parsenode(tokens[2]);
    }

    if ((s = tokens[3]) != NULL)
    {
        a.attrib.local = 1;
        a.attr = 1;
        while (*s)
        {
            switch (tolower(*s))
            {
            case 'p':
                a.attrib.priv = 1;
                break;
            case 'h':
                a.attrib.hold = 1;
                break;
            case 'd':
                a.attrib.direct = 1;
                break;
            case 'c':
                a.attrib.crash = 1;
                break;
            case 'k':
                a.attrib.killsent = 1;
                break;
            case 'n':
                a.attr = 0;
                break;
            default:
                /* unknown */
                break;
            }
            s++;
        }

        if (tokens[4] != NULL)
        {
            a.subj = xstrdup(tokens[4]);
        }
    }

    aliaslist = xrealloc(aliaslist, (++SW->otheraliases) * sizeof(struct _alias));
    aliaslist[SW->otheraliases - 1] = a;
}

/*
 *  Handles the areafiles.
 */

void do_areafile(char *value)
{
    char *tokens[3];

    memset(tokens, 0, sizeof tokens);
    parse_tokens(value, tokens, 2);

    if (tokens[0] == NULL)
    {
        return;
    }

    areas_type = AREAS_BBS;

    if (toupper(*tokens[0]) == 'S')
    {
        areas_type = SQUISH;
    }

    if (toupper(*tokens[0]) == 'F')
    {
        areas_type = FASTECHO;
    }

    if (tokens[1])
    {
        char *temp;
        temp = pathcvt(xstrdup(tokens[1]));
        if (areas_type == AREAS_BBS)
        {
            checkareas(temp);
        }
        else if (areas_type == FASTECHO)
        {
            check_fastecho(temp);
        }
        else
        {
            check_squish(temp);
        }
        release(temp);
    }
    else
    {
        /* no area-file was specified, we want to check later */
        check_area_files = 1;
    }
}

/*
 *  Compares two areas. Qsort helper function for areasort.
 */

#define compare(x,y) ( (x) < (y) ? -1 : ((x) == (y) ? 0 : 1) )

static int compare_areas(const void *x1, const void *x2)
{
  const AREA *a1 = (const AREA *)x1;
  const AREA *a2 = (const AREA *)x2;
  const char *cp;
  int retval = 0;


  for (cp = ST->sort_criteria; *cp; cp++)
  {
      switch(toupper(*cp))
      {
      case 'N':   /* sort netmail first, then local, then echomail */
          retval = compare(a1->netmail + a1->local * 2 + a1->echomail * 4,
                           a2->netmail + a2->local * 2 + a2->echomail * 4);
          break;
      case 'T':   /* sort by area tag */
          retval = strcmp(a1->tag, a2->tag);
          break;
      case 'D':   /* sort by area description */
          retval = strcmp(a1->description, a2->description);
          break;
      case 'G':   /* sort by fastecho group */
          retval = compare(a1->group, a2->group);
          break;
      }
      if (retval)
      {
         return retval;
      }
  }
  /* No differences found using the specified criteria. Maintain the
     original insertion order. */
  return compare(a1->areanumber, a2->areanumber);
}


/*
 *  Sorts the areas in the defined manner and sets the arenumber field.
 */

static void areasort(void)
{
    int i;

    for (i=0; i<SW->areas; i++)
    {
        arealist[i].areanumber=i;
    }

    if (ST->sort_criteria == NULL || SW->areas == 0)
    {
        return;
    }

    qsort(arealist, SW->areas, sizeof(AREA), compare_areas);

    for (i=0; i<SW->areas; i++)
    {
        arealist[i].areanumber=i;
    }
}



/*
 *  Handles an entire config file.
 */

static void parseconfig(FILE * fp)
{
    static char progress_indicators[4] =
    {'-', '\\', '|', '/'};
    static char raw_buffer[TEXTLEN];
    static char *buffer = NULL;
    char *keyword;
    char *value = NULL;
    char *s = NULL;
    char *tokens[20];
    int i = 0, line_num = 0;

    memset(raw_buffer, 0, TEXTLEN);

    while (!feof(fp))
    {
        line_num++;

        printf("%c\b", progress_indicators[line_num % 2]);
        fflush(stdout);

        if (fgets(raw_buffer, TEXTLEN, fp) == NULL)
        {
            return;
        }
        buffer = env_expand(raw_buffer); /* expand %ENVIRONMENT% variables */

        keyword = strtok(buffer, " \t\n\r");

        if (keyword)
        {
            strlwr(keyword);
        }
        else
        {
            continue;
        }

        if (*keyword == ';')
        {
            continue;
        }

        value = strtok(NULL, ";\n\r");
        value = striptwhite(value);

        /* clear the pointers in the array */

        memset(tokens, 0, sizeof(tokens));

        while (value && *value && isspace((unsigned char)*value))
        {
            if (*value == '\n' || *value == ';')
            {
                break;
            }
            else
            {
                value++;
            }
        }

        switch (GetVerbNum(keyword))
        {
        case -1:
            printf("\nUnknown configuration keyword: '%s'\n", keyword);
            break;

        case CFG_NAME:
            parse_tokens(value, tokens, 3);
            if (tokens[0] != NULL)
            {
                for (i = 0; i < 11; i++)
                {
                    if (user_list[i].name == NULL)
                    {
                        break;
                    }
                }
                if (i < 11)
                {
                    user_list[i].name = xstrdup(tokens[0]);

                    if (tokens[1] != NULL)
                    {
                        user_list[i].lastread = xstrdup(tokens[1]);
                    }

                    if (tokens[2] != NULL)
                    {
                        user_list[i].offset = atol(tokens[2]);
                        user_list[i].offs_defined = 1;
                    }

                    if (i == 0)
                    {
                        release(ST->username);
                        ST->username = xstrdup(user_list[i].name);

                        if (user_list[i].lastread)
                        {
                            release(ST->lastread);
                            ST->lastread = xstrdup(user_list[i].lastread);
                        }
                        SW->useroffset = user_list[i].offset;
                    }
                }
            }
            break;

        case CFG_ADDRESS:
            alias = xrealloc(alias, (++SW->aliascount) * sizeof(ADDRESS));
            alias[SW->aliascount - 1] = parsenode(value);
            break;

        case CFG_PRIVATENET:
            SW->pointnet = (int)strtol(value, NULL, 0);
            break;

        case CFG_ALIAS:
            parse_alias(value);
            break;

        case CFG_OUTFILE:
            release(ST->outfile);
            if (strchr("?+", value[0]) != NULL)
            {
                char *temp;
                temp = pathcvt(strdup(value + 1));
                ST->outfile = xmalloc(strlen(temp) + 2);
                ST->outfile[0] = value[0];
                ST->outfile[1] = '\0';
                strcat(ST->outfile, temp);
                release(temp);
            }
            else
            {
                ST->outfile = pathcvt(xstrdup(value));
            }
            break;

        case CFG_LASTREAD:
            release(ST->lastread);
            ST->lastread = xstrdup(value);
            for (i = 0; i < 11; i++)
            {
                if (user_list[i].name == NULL)
                {
                    break;
                }
                else if (user_list[i].lastread == NULL)
                {
                    user_list[i].lastread = xstrdup(value);
                }
            }
            break;

        case CFG_TOSSLOG:
            release(ST->echotoss);
            if (value!=NULL)
            {
                if (value[0] == '+')
                {
                    ST->echotoss = xstrdup(value + 1);
                }
                else
                {
                    ST->echotoss = xstrdup(value);
                }
            }
            break;

        case CFG_USERLIST:
            ST->fidolist = pathcvt(xstrdup(strtok(value, ",\n")));
            ST->userlist = strtok(NULL, ",\n");
            if (ST->userlist != NULL)
            {
                ST->userlist = pathcvt(xstrdup(ST->userlist));
            }
            break;

        case CFG_SWAPPATH:
            release(ST->swap_path);
            kill_trail_slash(value);
            ST->swap_path = pathcvt(xstrdup(value));
            break;

        case CFG_NODEPATH:
            release(ST->nodepath);
            kill_trail_slash(value);
            ST->nodepath = pathcvt(xstrdup(value));
            break;

        case CFG_NODELIST:
            parse_tokens(value, tokens, 3);
            if (tokens[2] != NULL)
            {
                node_lists = xrealloc(node_lists, (++SW->nodelists) * sizeof(D_LIST));
                node_lists[SW->nodelists - 1].name = xstrdup(tokens[0]);
                node_lists[SW->nodelists - 1].base_name = xstrdup(tokens[1]);
                node_lists[SW->nodelists - 1].sysop = xstrdup(tokens[2]);

                if (SW->nodelists == 1)
                {
                    ST->nodebase = node_lists[SW->nodelists - 1].base_name;
                    ST->sysop = node_lists[SW->nodelists - 1].sysop;
                }
            }
            break;

        case CFG_HELPFILE:
            release(ST->helpfile);
            ST->helpfile = pathcvt(xstrdup(value));
            break;

        case CFG_AREAFILE:
            do_areafile(value);
            break;

        case CFG_USEROFFSET:
            SW->useroffset = atoi(value);
            for (i = 0; i < 11; i++)
            {
                if (user_list[i].name == NULL)
                {
                    break;
                }
                else if (user_list[i].offs_defined == 0)
                {
                    user_list[i].offset = SW->useroffset;
                    user_list[i].offs_defined = 1;
                }
            }
            break;

        case CFG_QUOTE:
            release(ST->quotestr);
            striptwhite(value);
            ST->quotestr = xstrdup(value);
            s = ST->quotestr;
            while (*s)
            {
                if (*s == '_')
                {
                    *s = ' ';
                }
                s++;
            }
            break;

        case CFG_ALTERFUNC:
            parse_tokens(value, tokens, 2);
            if (tokens[0] && tokens[1])
            {
                int var = 0;
                char *c;

                c = striplwhite(tokens[0]);
                s = striplwhite(tokens[1]);

                if (*s != '\0' && *c != '\0')
                {
                    while (*c && !isspace((unsigned char)*c))
                    {
                        switch (toupper(*c))
                        {
                        case 'Q':
                            if (!(var & MT_REP) && !(var & MT_NEW))
                            {
                                var |= MT_QUO;
                            }
                            break;

                        case 'R':
                            if (!(var & MT_QUO) && !(var & MT_NEW))
                            {
                                var |= MT_REP;
                            }
                            break;

                        case 'N':
                            if (!(var & MT_QUO) && !(var & MT_REP))
                            {
                                var |= MT_NEW;
                            }
                            break;

                        case 'A':
                            var |= MT_ARC;
                            break;

                        case 'F':
                            var |= MT_FOL;
                            break;

                        default:
                            break;
                        }
                        c++;
                    }
                    if (!stricmp("ReplyQuote", s))
                    {
                        SW->rquote = var;
                    }
                    else if (!stricmp("ReplyOtherArea", s))
                    {
                        SW->rotharea = var;
                    }
                    else if (!stricmp("ReplyFollow", s))
                    {
                        SW->rfollow = var;
                    }
                    else if (!stricmp("ReplyExtra", s))
                    {
                        SW->rextra = var;
                    }
                }
            }
            break;

        case CFG_SWITCH:
            parse_tokens(value, tokens, 2);
            if (tokens[1] != NULL)
            {
                AssignSwitch(value, stricmp(tokens[1], "on") == 0 ? 1 : 0);
            }
            break;

        case CFG_COLOR:
        case CFG_COLOUR:
            parse_tokens(value, tokens, 3);
            if (tokens[2] != NULL)
            {
                int fcol, bcol;
                fcol = GetColor(tokens[1]);
                bcol = GetColor(tokens[2]);
                if (fcol == -1 || bcol == -1)
                {
                    continue;
                }
                AssignColor(tokens[0], fcol | bcol);
            }
            break;

        case CFG_RIGHT:
            SW->rm = (int)strtol(value, NULL, 0);
            break;

        case CFG_QUOTERIGHT:
            SW->qm = (int)strtol(value, NULL, 0);
            break;

        case CFG_TABSIZE:
            SW->tabsize = (int)strtol(value, NULL, 0);
            break;

        case CFG_CURSTART:
            cur_start = (int)strtol(value, NULL, 0);
            break;

        case CFG_CUREND:
            cur_end = (int)strtol(value, NULL, 0);
            break;

        case CFG_FIDO:
        case CFG_SQUISH:
        case CFG_QUICK:
            parsemail(keyword, value);
            break;

        case CFG_UUCP:
            uucp_gate = parsenode(value);
            break;

        case CFG_DOMAIN:
            domain_list = xrealloc(domain_list, (++SW->domains) * sizeof(ADDRESS));
            domain_list[SW->domains - 1] = parsenode(value);
            break;

        case CFG_GATE:
            SW->gate = 0;
            switch (toupper(*value))
            {
            case 'Z':  /* Zones */
                SW->gate = GZONES;
                break;
            case 'D':  /* Domains */
                SW->gate = GDOMAINS;
                break;
            case 'B':  /* Both */
                SW->gate = BOTH;
                break;
            default:
                break;
            }
            break;

        case CFG_ORIGIN:
            if (value != NULL)
            {
                if (n_origins == 0)
                {
                    origins = xmalloc(sizeof(char*) * (n_origins = 1));
                }
                else
                {
                    origins = xrealloc(origins, sizeof(char*) * (++n_origins));
                }

                origins[n_origins - 1] = xstrdup(value);
                release(ST->origin);
                ST->origin = xstrdup(value);
            }
            break;

        case CFG_READKEY:
        case CFG_EDITKEY:
            if (value)
            {
                strlwr(value);
                i = (int)strtol(value, &value, 0);
                value = striplwhite(value);
                if (*keyword == 'e')
                {
                    e_assignkey(i, value);
                }
                else
                {
                    r_assignkey(i, value);
                }
            }
            break;

        case CFG_FUNCTION:
            i = (int)strtol(value, &s, 0);
            s = striplwhite(s);
            if (i >= 0 && i <= 40)
            {
                macros[i] = parse_macro(s);
            }
            break;

        case CFG_INCLUDE:
            if (value)
            {
                FILE *ifp;
                char *temp;
                temp = pathcvt(xstrdup(value));
                ifp = fopen(temp, "r");
                release(temp);
                if (ifp != NULL)
                {
                    parseconfig(ifp);
                    fclose(ifp);
                }
            }
            break;

        case CFG_MAXX:
            maxx = (int)strtol(value, NULL, 0);
            break;

        case CFG_MAXY:
            maxy = (int)strtol(value, NULL, 0);
            break;

        case CFG_TEMPLATE:
            templates = xrealloc(templates, (++SW->numtemplates) * sizeof(char *));
            templates[SW->numtemplates - 1] = pathcvt(xstrdup(value));
            if (SW->numtemplates == 1)
            {
                ST->template = xstrdup(templates[SW->numtemplates - 1]);
            }
            break;

        case CFG_UUCPNAME:
            release(ST->uucpgate);
            ST->uucpgate = xstrdup(value);
            break;

        case CFG_GROUP:
            group = xrealloc(group, (++num_groups) * sizeof(GROUP));
            parse_tokens(value, tokens, 3);
            if (tokens[2] != NULL)
            {
                group[num_groups - 1].search = xstrdup(strupr(tokens[0]));
                group[num_groups - 1].username = atoi(tokens[1]);

                if ((group[num_groups - 1].username >= MAXUSERS) ||
                  (group[num_groups - 1].username >= MAXUSERS) ||
                  (user_list[group[num_groups - 1].username].name == NULL))
                {
                    group[num_groups - 1].username = 0;
                }

                group[num_groups - 1].template = atoi(tokens[2]);
                if (group[num_groups - 1].template >= SW->numtemplates)
                {
                    group[num_groups - 1].template = 0;
                }
            }
            break;

        case CFG_EDITOR:
            release(ST->editorName);
            ST->editorName = pathcvt(xstrdup(value));
            break;

        case CFG_ROBOTNAME:
            parse_tokens(value, tokens, 3);
            if (tokens[0] != NULL)
            {
                for (i = 0; i < 11; i++)
                {
                    if (user_list[i].robotname == NULL)
                    {
                        break;
                    }
                }
                if (i < 11)
                {
                    user_list[i].robotname = xstrdup(tokens[0]);
                }
            }
            break;

        case CFG_QUICKBBS:
            release(ST->quickbbs);
            kill_trail_slash(value);
            ST->quickbbs = pathcvt(xstrdup(value));
            break;

        case CFG_SCAN:
            scan = 1;
            break;

        case CFG_MOUNTDIR:
            parse_tokens(value, tokens, 2);
            if (tokens[0] != NULL && tokens[1] != NULL)
            {
                mntdirunix = xstrdup(tokens[0]);
                mntdirdos = xstrdup(tokens[1]);
            }
            break;

        case CFG_SOFTCRXLAT:
            softcrxlat = (int)strtol(value, NULL, 0);
            break;

        case CFG_AREAEXCL:
            SkipArea(value);
            break;

        case CFG_OUTPUTCHARSET:
            release(ST->output_charset);
            ST->output_charset = xstrdup(value);
            break;

        case CFG_SORTAREAS:
            release(ST->sort_criteria);
            ST->sort_criteria = xstrdup(value);
            break;

        case CFG_ENABLESC:
            TTEnableSCInput(value);
            break;

        case CFG_AREAFILEFLAGS:
            if (value != NULL)
            {
                if (areafileflags == NULL)
                {
                    areafileflags = xstrdup(value);
                }
                else
                {
                    areafileflags = xrealloc(areafileflags,
                                             strlen(areafileflags) +
                                             strlen(value) + 1 );
                    s = areafileflags + strlen(areafileflags);
                    
                    for (; *value; value++)
                    {
                        if (strchr(areafileflags, *value) == NULL)
                        {
                            *s++ = *value;
                            *s = '\0';
                        }
                    }
                }
            }
            break;

        case CFG_FREQAREA:
            if (ST->freqarea != NULL)
            {
                xfree(ST->freqarea);
            }
            ST->freqarea = xstrdup(value);
            break;

        default:
            printf("\nUnknown configuration keyword: '%s'\n", keyword);
            break;
        }
        release(buffer);
        memset(raw_buffer, 0, TEXTLEN);
    }
}

void show_debuginfo(int macro_count)
{
    char *szYes = "Yes";
    char *szNo = "No";

    MouseOFF();

    printf(
      "\n"
      "%-30s; %s\n"
      "-------------------------------------------------------------------------------\n"
      "\n",
      PROG " " VERSION CLOSED "; Mail Reader",
      "Compiled on " __DATE__ " at " __TIME__
    );

    printf("Screen size       : %d columns, %d rows\n", maxx, maxy);
    printf("User              : \"%s\" (%s)\n", ST->username, show_address(&thisnode));
    printf("Origin            : \"%s\"\n", ST->origin);
    printf("Macros            : %d macros defined\n", macro_count);
    printf("Home directory    : %s\n", ST->home);
    printf("Quote string      : \"%s\"\n", ST->quotestr);
    printf("Export file       : %s (default)\n", ST->outfile);
    printf("Config file       : %s\n", ST->cfgfile);
    printf("Echotoss log      : %s\n", ST->echotoss);
    printf("Template file     : %s\n", ST->template);
#ifdef MSDOS
    printf("Swap path         : %s\n", ST->swap_path);
#endif
    printf("Help file         : %s\n", ST->helpfile);
    printf("Command processor : %s\n", ST->comspec);
    printf("External editor   : %s\n", ST->editorName);
    printf("QuickBBS path     : %s\n", ST->quickbbs);
    printf("\n");
    printf("Areas                     : %d area%s configured\n", SW->areas, SW->areas == 1 ? "" : "s");
    printf("Generate MSGIDs           : %s\n", SW->msgids ? szYes : szNo);
    printf("Generate Opus time stamps : %s\n", SW->opusdate ? szYes : szNo);
    printf("Show hidden lines         : %s\n", SW->shownotes ? szYes : szNo);
    printf("Show SEEN-BY lines        : %s\n", SW->showseenbys ? szYes : szNo);
    printf("Show origin lines         : %s\n", SW->showorigins ? szYes : szNo);
    printf("Show tearlines            : %s\n", SW->showtearlines ? szYes : szNo);
    printf("Confirm deletes, aborts   : %s\n", SW->confirmations ? szYes : szNo);
    printf("Show date arrived         : %s\n", SW->datearrived ? szYes : szNo);
    printf("Show current address      : %s\n", SW->showaddr ? szYes : szNo);
    printf("Use lastread/current      : %s\n", SW->use_lastr ? szYes : szNo);
    printf("Quote quotes              : %s\n", SW->qquote ? szYes : szNo);
    printf("Save original carbon copy : %s\n", SW->savecc ? szYes : szNo);
    printf("Save raw carbon copy      : %s\n", SW->rawcc ? szYes : szNo);
    printf("Chop end of quoted msgs   : %s\n", SW->chopquote ? szYes : szNo);
    printf("Use \"HardQuotes\" feature  : %s\n", SW->hardquote ? szYes : szNo);
    printf("Show paragraph markers    : %s\n", SW->showcr ? szYes : szNo);
    printf("Show end-of-line markers  : %s\n", SW->showeol ? szYes : szNo);
    printf("Show real message numbers : %s\n", SW->showrealmsgn ? szYes : szNo);
    printf("Enable mouse support      : %s\n", SW->usemouse ? szYes : szNo);
    printf("Expand tab character      : %s\n", SW->tabexpand ? szYes : szNo);
    printf("Only show CRs in editor   : %s\n", SW->editcronly ? szYes : szNo);
    printf("Generate PID              : %s\n", SW->usepid ? szYes : szNo);
    printf("Generate echo tearlines   : %s\n", SW->usetearlines ? szYes : szNo);
    printf("Generate echo origins     : %s\n", SW->useoriginlines ? szYes : szNo);
    printf("Generate SOT/EOT          : %s\n", SW->soteot ? szYes : szNo);
    printf("Show system time          : %s\n", SW->showtime ? szYes : szNo);
    printf("Show file hdrs on import  : %s\n", SW->importfn ? szYes : szNo);
    printf("Msg num at top of screen  : %s\n", SW->dmore ? szYes : szNo);
    printf("Show status bar           : %s\n", SW->statbar ? szYes : szNo);
    printf("Perform address lookup    : %s\n", SW->showsystem ? szYes : szNo);
    printf("Format external messages  : %s\n", SW->extformat ? szYes : szNo);
    printf("Exact match in area list  : %s\n", SW->arealistexactmatch ? szYes : szNo);
    printf("Generate echomail FLAGS   : %s\n", SW->echoflags ? szYes : szNo);
    printf("Generate netmail Via      : %s\n", SW->netmailvia ? szYes : szNo);
    printf("Domain in origin line     : %s\n", SW->domainorigin ? szYes : szNo);
    printf("Key right to next unread  : %s\n", SW->rightnextunreadarea ? szYes : szNo);

    exit(0);
}

/*
 *  Handles the entire configuration process.
 */

void opening(char *cfgfile, char *areafile)
{
    WND *hCurr, *hWnd;
    FILE *fp;
    int count = 0, i;
    static char cfnname[] = "msged.cfg";
    char tmp[PATHLEN];

    InitVars();

    printf(PROG " " VERSION CLOSED " ... ");
    fflush(stdout);

    read_charset_maps(); /* initialise the FTSC0054 charset engine */

    fp = fileopen(cfnname, cfgfile);
    if (fp == NULL)
    {
        if (cfgfile == NULL)
        {
            cfgfile = cfnname;
        }
        printf("\nCannot open " PROG " configuration file '%s'.\n", cfgfile);
        exit(-1);
    }

    parseconfig(fp);
    fclose(fp);

    if (alias == NULL)
    {
        printf("\nError! Primary address must be defined.\n");
        exit(-1);
    }

    if (user_list[0].name == NULL)
    {
        printf("\nError! Name must be defined.\n");
        exit(-1);
    }

    for (i = 0; i < 40; i++)
    {
        count += macros[i] != NULL ? 1 : 0;
    }

    if (check_area_files)
    {
        if (areas_type == AREAS_BBS)
        {
            checkareas(areafile);
        }
        else if (areas_type == FASTECHO)
        {
            check_fastecho(areafile);
        }
        else
        {
            check_squish(areafile);
        }
    }

    if (arealist == NULL)
    {
        printf("\nError! At least one message area must be defined.\n");
        exit(-1);
    }

    if (SW->soteot && SW->domainorigin)
    {
        printf("\nError! The SOT/EOT specification does not permit domains in "
        "echomail origin\nlines. Please set either \"Switch DomainOrigin\" or "
        "\"Switch SOTEOT\" to \"Off\".\n");
        exit(-1);
    }

    areasort();
    printf (" \n");
    InitScreen();
    mygetcwd(tmp, PATHLEN);
    ST->home = xstrdup(tmp);

    if (cmd_dbginfo)
    {
        show_debuginfo(count);
    }

    cursor(0);

    SW->rm = SW->rm > maxx ? maxx-1 : SW->rm;
    SW->qm = SW->qm > SW->rm - strlen(ST->quotestr) ?
                             SW->rm - strlen(ST->quotestr) : SW->qm;

    hCurr = WndTop();
    hWnd = WndPopUp(42, 6, SBDR | SHADOW, cm[IN_BTXT], cm[IN_NTXT]);

    WndPutsCen(0, cm[IN_NTXT], PROG " Mail Reader");
    WndPutsCen(2, cm[IN_BTXT], "Version " VERSION CLOSED);
    WndPutsCen(4, cm[IN_NTXT], "Press a key to continue");

    GetKey();

    WndClose(hWnd);
    WndCurr(hCurr);
    TTScolor(cm[CM_NTXT]);
    TTClear(hMnScr->x1, hMnScr->y1, hMnScr->x2, hMnScr->y2);
}
