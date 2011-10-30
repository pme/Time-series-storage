/* vim: set ts=2 sw=2:
 * =====================================================================================
 *
 *       Filename:  pr.c
 *
 *    Description:  Pipe reader
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

#include "pr.h"
#include "ic.h"
#include "mh.h"

#define PATH "./data"
#define FILES 256
#define CACHE_SIZE 20

int sig;

void sighandler(int signum)
{
  sig = signum;
}

struct m * lineparser(char *line)
{
	static struct m m;
	char *p;

	strncpy(m.id, strtok(line, "|"), 16);
  m.id[16] = '\0';
	m.ts.tv_sec  = atoi(strtok(NULL, "."));
	m.ts.tv_usec = atoi(strtok(NULL, "|"));
	m.val        = atof(strtok(NULL, "|"));
	m.stat       = strtoul(strtok(NULL, "|") , &p, 16);

	return &m;
}

void printm(FILE *f, char *data)
{ 
	struct m *m = (struct m *)data;

	fprintf(f, "'%s' %9ld.%-9ld %10.3f 0x%03x\n",
			m->id,
			m->ts.tv_sec,
			m->ts.tv_usec,
			m->val,
			m->stat);
}

int main (int argc, char *argv[])
{
  int opt, trace;
  char *pn;
	FILE *f;
	char buf[PIPE_BUF];
	char *l;
  struct ic *ic;
  int i;

	trace = 0;
	while ((opt = getopt(argc, argv, "th")) != -1) {
		switch (opt) {
			case 't':
				trace = 1;
				break;
			default: /* '?' */
				fprintf(stderr, "Usage: %s [-t] pipename\n", basename(argv[0]));
				fprintf(stderr, "\t-t - Trace\n");
				exit(EXIT_FAILURE);
		}
	}
	if (optind >= argc) {
		fprintf(stderr, "Pipename expected after options\n");
		exit(EXIT_FAILURE);
	}

  pn = argv[optind];

  /* ------------------------------------------------------ */

	if ((f = fopen(pn, "rw+")) == NULL)
		err(EXIT_FAILURE, "fopen(%s)", pn);

	signal(SIGALRM, sighandler);
	alarm(1);

  if (mkdir(PATH, 0755) == -1 && errno != EEXIST)
    err(EXIT_FAILURE, "mkdir(%s)", PATH);

  ic = ICcreate(FILES, sizeof(struct m), CACHE_SIZE, PATH);

	while(1) {
		if ((l = fgets(buf, PIPE_BUF, f))) {
			struct m *m;

      buf[strlen(buf)-1] = '\0';

			if (trace) fprintf(stdout, "line: %s\n", buf);

			m = lineparser(buf);
			if (trace) printm(stdout, (char *)m);

		  i = ICadd(ic, hash(m->id), (char *)&m);

      ICprintcache(stderr, ic, 1, printm);

		} else if (l == NULL && errno == EINTR) {
			if (sig) {
        time_t t = time(NULL);

				if (trace) fprintf(stdout, "signal: %d %s", sig, ctime(&t));

        i = ICflushall(ic);

				sig = 0;
				signal(SIGALRM, sighandler);
				alarm(1);
			}
		}
	}

  i = ICdrop(ic);

	return 0;
}
