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
#include <assert.h>

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
	fprintf(stderr, "Usage: %s [-h] [-v] [-i id] [-f from] [-t to] [-H]\n", basename(argv[0]));
	fprintf(stderr, "\t-h - Print this message to stdout\n");
	fprintf(stderr, "\t-v - Verbose\n");
	fprintf(stderr, "\t-i - Channel identifier\n");
	fprintf(stderr, "\t-f - from timestamp\n");
	fprintf(stderr, "\t-t - to timestamp\n");
	fprintf(stderr, "\t-H - human readable output\n");
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

void dumpfile(FILE *out, char *fname, char *id, time_t fts, time_t tts, int human)
{
  int fd;
  struct m m;
  char path[132+1];

  snprintf(path, 132, PATH "/%s", fname);
  fd = open(path, O_RDONLY|O_LARGEFILE);
	assert(fd != -1);

#ifdef USE_FLOCK
  if (flock(fd, LOCK_SH) < 0) {
		fprintf(stderr, "get shared lock failure\n");
    close(fd);
    return;
	}
#endif

  while (readn(fd, &m, sizeof(m))) {
		if (!strcmp(id, m.id) &&
			m.ts.tv_sec >= fts &&
			m.ts.tv_sec <= tts) printm(out, &m, human);
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

	to = fm = 0;
	while ((opt = getopt(argc, argv, "i:f:t:vHh")) != -1) {
		switch (opt) {
			case 'i':
				id = optarg;
				break;
			case 't':
				to = optarg;
				break;
			case 'f':
				fm = optarg;
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
  if (id == NULL || fm == NULL || to == NULL)
		usage(argc, argv);

  /* ------------------------------------------------------ */

  idhash = hash(id);
  snprintf(idpattern, PATTERN_LEN, "_%03d.dat", idhash);

  fmpattern = space2underline(fm);
  topattern = space2underline(to);

  fts = strtimetotv(fm);
  tts = strtimetotv(to);

	if (verbose)
		fprintf(stderr, "'%s' -> %03d -- %s <> %s\n", id, idhash, fmpattern, topattern);

  dn = scandir(PATH, &de, filter, versionsort);

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

  free(de);

	return 0;
}
