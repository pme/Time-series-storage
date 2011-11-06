/* vim: set ts=2 sw=2:
 * =====================================================================================
 *
 *       Filename:  dg.c
 *
 *    Description:  Data generator for test purpouses.
 *
 *        Version:  1.0
 *        Created:  2011-07-31 15:16
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
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <libgen.h>
#include <sys/time.h>
#include <time.h>
#include <err.h>
#include <string.h>
#include <math.h>
#include <string.h>

char **idname;

void usage(int argc, char *argv[])
{
	errx(EXIT_FAILURE, "Usage: %s [-n number of time] [-s step in microsec] [-S state]\n\
    id(s) will be read from stdin\n", basename(argv[0]));
}

int readids(void)
{
	int idcnt = 0;
	char line[1024];

	while ((fgets(line, 1024, stdin)) != NULL) {
		if (line[0] == '#') continue;
		idname = (char **)realloc(idname, (idcnt+1) * sizeof(char *));
		line[strlen(line)-1] = '\0';
		idname[idcnt++] = strdup(line);
	}
	return idcnt;
}

int main(int argc, char *argv[])
{
	int i, opt, idnum = 0;
	double st = -1.0;
  struct tm frtm, totm;
  struct timeval frtv, totv;
  double t;
  int s = 0, n = 0;
	char to[64], fm[64];

	while ((opt = getopt(argc, argv, "n:s:S:h")) != -1) {
		switch (opt) {
			case 'n':
				n = atoi(optarg);
				break;
			case 's':
				st = atof(optarg);
				break;
			case 'S':
				s = atoi(optarg);
				break;
			case 'h':
			default: /*  '?' */
				usage(argc, argv);
		}
	}

	idnum = readids();

  if (idnum == 0 || n == 0 || st < 0.0)
		usage(argc, argv);

  gettimeofday(&frtv, NULL);

  memcpy(&frtm, gmtime(&frtv.tv_sec), sizeof(frtm));
  strftime(fm, sizeof(fm), "%Y-%m-%d %T", &frtm);

	totv.tv_sec  = frtv.tv_sec + (n * st / 1000000.0);
  totv.tv_usec = 0;

  memcpy(&totm, gmtime(&totv.tv_sec), sizeof(totm));
  strftime(to, sizeof(to), "%Y-%m-%d %T", &totm);

	for(i=0; i<idnum; i++)
		fprintf(stderr, "id[%5d] == '%s'\n", i, idname[i]);
	fprintf(stderr, "from: '%s.%06ld' to: '%s.%06ld' step: '%f'microsec %d times\n",
			fm, frtv.tv_usec, to, totv.tv_usec, st, n);

  t = frtv.tv_sec;
  st /= 1000000.0;
  while(t < totv.tv_sec) {
		for(i=0; i<idnum; i++)
      printf("%s|%.6f|%+.3f|%x\n", idname[i], t, sin(t)+cos(t), s);
    t += st;
  }

	for(i=0; i<idnum; i++)
		free(idname[i]);

	free(idname);

  return 0;
}
