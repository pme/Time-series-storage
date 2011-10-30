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

#define _XOPEN_SOURCE
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

#include "pr.h"
#include "mh.h"

#define PATH "./data"
#define FILES 256
#define CACHE_SIZE 20
#define PATTERN_LEN 32

char idpattern[PATTERN_LEN+1];
char *fmpattern;
char *topattern;

void printm(FILE *f, struct m *m)
{
	fprintf(f, "'%s' %9ld.%-9ld %10.3f 0x%03x\n",
			m->id,
			m->ts.tv_sec,
			m->ts.tv_usec,
			m->val,
			m->stat);
}

int filter(const struct dirent *de)
{
  const char *s = de->d_name;

  /* fprintf(stderr, "f: '%s' id: '%s' fm: '%s' to: '%s' --> ", s, idpattern, fmpattern, topattern); */

  if (strstr(s, idpattern) == NULL) { /* other channel (file) or */
    /* fprintf(stderr, "OTHER\n"); */
    return 0;
  }
  if (strcmp(s, fmpattern) == -1)    { /* older or */
    /* fprintf(stderr, "OLDER\n"); */
    return 0;
  }
  if (strcmp(s, topattern) ==  +1)    { /* newer */
    /* fprintf(stderr, "NEWER\n"); */
    return 0;
  }
  /* fprintf(stderr, "OK\n"); */

  return 1;
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
	fprintf(stderr, "Usage: %s [-i id] [-f from] [-t to]\n", basename(argv[0]));
	fprintf(stderr, "\t-i - Channel identifier\n");
	fprintf(stderr, "\t-f - from timestamp\n");
	fprintf(stderr, "\t-t - to timestamp\n");
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

void dumpfile(FILE *out, char *fname, char *id)
{
  int fd;
  struct m m;
  char path[132+1];

  snprintf(path, 132, PATH "/%s", fname);
  fd = open(path, O_RDONLY);

  while (readn(fd, &m, sizeof(m))) {
    printm(out, &m);
  }

  close(fd);
}

int main (int argc, char *argv[])
{
  int opt;
  char *id = NULL, *fm = NULL, *to = NULL;
  struct dirent **de = NULL;
  int dn, i, idhash;

	to = fm = 0;
	while ((opt = getopt(argc, argv, "i:f:t:h")) != -1) {
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

  fprintf(stderr, "'%s' -> %03d -- %s <> %s\n", id, idhash, fmpattern, topattern);
  dn = scandir(PATH, &de, filter, versionsort);

  for (i=0; i<dn; i++) {
    fprintf(stderr, "%d - '%s'\n", i, de[i]->d_name);

    dumpfile(stdout, de[i]->d_name, id);
  }

	return 0;
}
