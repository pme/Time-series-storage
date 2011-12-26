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
#include <assert.h>

#include "pr.h"
#include "ic.h"
#include "mh.h"

#define PATH "./data"
#define FILES 256
#define CACHE_SIZE 20

int sigflush;
int sigopen;

void alarmhandler(int signum)
{
  sigflush = signum;
}

void openhandler(int signum)
{
  sigopen = signum;
}

struct m * lineparser(char *line)
{
	static struct m m;
	char *p;

	if ((p = strtok(line, "|")) == NULL) return NULL;
	strncpy(m.id, p, 16);
  m.id[16] = '\0';

  if ((p = strtok(NULL, ".")) == NULL) return NULL;
	m.ts.tv_sec = atoi(p);

  if ((p = strtok(NULL, "|")) == NULL) return NULL;
	m.ts.tv_usec = atoi(p);

  if ((p = strtok(NULL, "|")) == NULL) return NULL;
	m.val = atof(p);

  if ((p = strtok(NULL, "|")) == NULL) return NULL;
	m.stat = strtoul(p, NULL, 16);

	return &m;
}

void printm(FILE *f, char *data)
{ 
	struct m *m = (struct m *)data;

	fprintf(f, "'%s' %9ld.%06ld %10.3f 0x%03x\n",
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
	int s;

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

	signal(SIGALRM, alarmhandler);
	alarm(1);

	signal(SIGUSR1, openhandler);

  if (mkdir(PATH, 0755) == -1 && errno != EEXIST)
    err(EXIT_FAILURE, "mkdir(%s)", PATH);

  ic = ICcreate(FILES, sizeof(struct m), CACHE_SIZE, PATH);
	assert(ic);

	while (1) {

		if ((l = fgets(buf, PIPE_BUF, f))) {
			struct m *m;

      buf[strlen(buf)-1] = '\0';

			if (trace) fprintf(stdout, "line: %s\n", buf);

			if ((m = lineparser(buf)) == NULL) continue;
			if (trace) printm(stdout, (char *)m);

		  s = ICadd(ic, hash(m->id), (char *)m);
			assert(s == IC_OK);

      if (trace) ICprintcache(stderr, ic, 1, printm);

		} else if (l == NULL && errno == EINTR) {
      time_t t = time(NULL);

			if (sigflush) {

				if (trace) fprintf(stdout, "signal: %d %s", sigflush, ctime(&t));

        s = ICflushall(ic);
			  assert(s == IC_OK);

				sigflush = 0;
				signal(SIGALRM, alarmhandler);
				alarm(1);
			}

			if (sigopen) {

				if (trace) fprintf(stdout, "signal: %d %s", sigopen, ctime(&t));

        s = ICdrop(ic);
			  assert(s == IC_OK);

        ic = ICcreate(FILES, sizeof(struct m), CACHE_SIZE, PATH);
			  assert(ic);

				sigopen = 0;
				signal(SIGUSR1, openhandler);
			}
		} else {
			break; /* while (1) */
		}
	}

  s = ICdrop(ic);
	assert(s == IC_OK);

	return 0;
}
