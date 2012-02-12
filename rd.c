/* vim: set ts=2 sw=2:
 * =====================================================================================
 *
 *       Filename:  rd.c
 *
 *    Description:  Data reader
 *
 *        Version:  1.0
 *        Created:  2011-07-16 22:43
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Peter Meszaros (pme), hauptadler@gmail.com
 *        Company:  Infalk Co.
 *
 * =====================================================================================
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * =====================================================================================
 */

#define _XOPEN_SOURCE	500
#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <libgen.h>
#include <err.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/file.h>

#include "pr.h"
#include "mh.h"

#define PATH "./data"
#define FILES 256
#define CACHE_SIZE 20
#define PATTERN_LEN 32

int verbose;
char idpattern[PATTERN_LEN+1];
char *fmpattern;
char *topattern;
int human;
char *datapath = PATH;

char epochstr[] = "1970-01-01 00:00:00";

void printm(FILE *f, struct m *m, int human)
{
	if (human) {
	  char ts[64+1];

	  strftime(ts, 64, "%Y-%m-%d %T", gmtime(&m->ts.tv_sec));

		fprintf(f, "%s '%s.%06ld' %10.3f 0x%03x\n",
				m->id,
				ts, m->ts.tv_usec,
				m->val,
				m->stat);
	} else {
		fprintf(f, "%s|%9ld.%06ld|%10.3f|0x%03x\n",
				m->id,
				m->ts.tv_sec, m->ts.tv_usec,
				m->val,
				m->stat);
	}
}

int filter(const struct dirent *de)
{
  return (strstr(de->d_name, idpattern) == NULL) ? 0 : 1;
}

char *space2underline(char *s)
{
  char *p = s;

  while(*p) {
    if (*p == ' ') *p = '_';
    p++;
  }
  return s;
}

void usage(int argc, char *argv[])
{
	fprintf(stderr, "Usage: %s [-h] [-v] [-H] [-p datapath]\n", basename(argv[0]));
	fprintf(stderr, "\t-h - Print this message to stdout\n");
	fprintf(stderr, "\t-v - Verbose\n");
	fprintf(stderr, "\t-H - human readable output\n");
	fprintf(stderr, "\t-p - datapath\n");
	fprintf(stderr, "\tstdin format:\n");
	fprintf(stderr, "\t00Jigqdb0KzK|2012-01-10 11:30:00|2012-01-10 12:00:00\n");
	fprintf(stderr, "\t-identifier-|--from timestamp---|---to timestamp----\n");

	exit(EXIT_FAILURE);
}

/* Read "n" bytes from a descriptor. */
ssize_t readn(int fd, void *vptr, size_t n)
{
	size_t nleft, nread;
	char	*ptr;

	ptr = vptr;
	nleft = n;
	while (nleft > 0) {
		if ((nread = read(fd, ptr, nleft)) < 0)
			return nread;		/* error, return < 0 */
		else if (nread == 0)
			break;				/* EOF */

		nleft -= nread;
		ptr   += nread;
	}
	return n - nleft;		/* return >= 0 */
}

/*
 * fname == NULL -- just return
 * id    == NULL -- all id
 * fts   == 0    -- from the epoch
 * tts   == 0    -- till now and even the future ;)
 */
void dumpfile(FILE *out, char *fname, char *id, time_t fts, time_t tts, int human)
{
  int fd;
  struct m m;
  char path[132+1];

  if (!fname) return;

  snprintf(path, 132, "%s/%s", datapath, fname);
  if ((fd = open(path, O_RDONLY|O_LARGEFILE)) == -1) 
	  err(EXIT_FAILURE, "open(%s)", path);

#ifdef USE_FLOCK
  if (flock(fd, LOCK_SH) < 0) {
		fprintf(stderr, "get shared lock failure\n");
    close(fd);
    return;
	}
#endif

  while (readn(fd, &m, sizeof(m))) {
		if ((!id  || !strcmp(id, m.id)) &&
			  (!fts || (m.ts.tv_sec >= fts)) &&
			  (!tts || (m.ts.tv_sec <= tts))) printm(out, &m, human);
  }

#ifdef USE_FLOCK
  if (flock(fd, LOCK_UN) < 0) {
		fprintf(stderr, "release shared lock failure\n");
	}
#endif

  close(fd);
}

time_t strtimetotv(char *strtime)
{
	struct tm tm;

	strptime(strtime, "%Y-%m-%d_%T", &tm);
	return timegm(&tm);
}

int main (int argc, char *argv[])
{
  int opt;
  char *id = NULL, *fm = NULL, *to = NULL;
	time_t fts, tts;
  struct dirent **de = NULL;
  int dn, i, idhash;
	char line[256];
	char *p;

	to = fm = 0;
	while ((opt = getopt(argc, argv, "p:vHh")) != -1) {
		switch (opt) {
			case 'p':
				datapath = optarg;
				break;
			case 'v':
				verbose = 1;
				break;
			case 'H':
				human = 1;
				break;
			case 'h':
			default: /* '?' */
        usage(argc, argv);
		}
	}

	for (i=optind; i<argc; i++) {
    dumpfile(stdout, argv[i], NULL, 0, 0, 0);
	}
	if (optind < argc) exit(EXIT_SUCCESS);

	if ((fgets(line, 256, stdin)) == NULL) usage(argc, argv);
  if ((p = strrchr(line, '\n'))) *p = '\0';
	if ((id = strtok(line, "|")) == NULL) usage(argc, argv);
	if ((fm = strtok(NULL, "|")) == NULL) usage(argc, argv);
	if ((to = strtok(NULL, "|")) == NULL) usage(argc, argv);
	if (*fm == '0') fm = epochstr;
	if (*to == '0') to = epochstr;

  /* ------------------------------------------------------ */

  idhash = hash(id);
  snprintf(idpattern, PATTERN_LEN, "_%03d.dat", idhash);

  fmpattern = space2underline(fm);
  topattern = space2underline(to);

  fts = strtimetotv(fm);
  tts = strtimetotv(to);

	if (verbose)
		fprintf(stderr, "'%s' directory: '%s' -> %03d -- %s/%ld <> %s/%ld\n",
		    datapath, id, idhash, fmpattern, fts, topattern, tts);

  dn = scandir(datapath, &de, filter, versionsort);
	if (dn < 0) err(EXIT_FAILURE, "scandir(%s)", datapath);

  char *prev = NULL;

  for (i=0; i<dn; i++) {
		if (verbose)
			fprintf(stderr, "%d - '%s'\n", i, de[i]->d_name);

		if (strcmp(de[i]->d_name, fmpattern) == -1) { /* older */
			prev = de[i]->d_name;  /* store the name, because we need the file right before the queried interval */
			continue;
		} else if (strcmp(de[i]->d_name, topattern) ==  +1) { /* newer */
			break; /* for() */
		}

    if (prev) { /* begin the queried interval with the previous file, do it only once! */
      dumpfile(stdout, prev, id, fts, tts, human);
			prev = NULL;
		}

    dumpfile(stdout, de[i]->d_name, id, fts, tts, human);
  }
	if (prev) { /* print the nonprinted prev if there has not been found other (newer) files */
		dumpfile(stdout, prev, id, fts, tts, human);
	}

  while (dn--)
		free(de[dn]);
  free(de);

	return 0;
}
