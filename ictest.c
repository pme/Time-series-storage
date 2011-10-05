/* vim: set ts=2 sw=2:
 * =====================================================================================
 *
 *       Filename:  ictest.c
 *
 *    Description:  ItemCache testing program
 *
 *        Version:  1.0
 *        Created:  2011-10-05 12:30
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
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <limits.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <string.h>
#include <err.h>
#include <libgen.h>
#include <stdint.h>

#include "ic.h"

#define PATH "./ic.test"
#define FILES 256
#define CACHE_SIZE 20

struct test_item {
  uint32_t id;
  struct timeval ts;
  float value;
  uint8_t state;
};

int main(int argc, char *argv[])
{
  int delete, opt;
  struct ic *ic;
  struct test_item ti;
  int i;

	delete = 0;
	while ((opt = getopt(argc, argv, "dh")) != -1) {
		switch (opt) {
			case 'd':
				delete = 1;
				break;
			default: /* '?' */
				fprintf(stderr, "Usage: %s [-d]\n", basename(argv[0]));
				fprintf(stderr, "\t-d - Delete test data direcrory\n");
				exit(EXIT_FAILURE);
		}
	}

  /* ------------------------------------------------------ */

  if (mkdir(PATH, 0755) == -1)
    err(EXIT_FAILURE, "mkdir(%s)", PATH);

  ic = ICcreate(FILES, sizeof(struct test_item), CACHE_SIZE, PATH);

	for (i=0; i<(FILES*(CACHE_SIZE+1)); i++) {
		ti.id = i;
		gettimeofday(&ti.ts, NULL);
		ti.value = i*3;
		ti.state = i+1;

		ICadd(ic, i % FILES, (char *)&ti);
	}

  ICprintcache(stdout, ic, 1);

  ICdrop(ic);

  if (delete && (system("rm -rf " PATH) == -1))
    err(EXIT_FAILURE, "system(\"rm -rf %s\")", PATH);

  return 0;
}

#if 0

struct ic * ICcreate(int nfiles, int itemsize, int nitem, char *path);
ssize_t ICflush(struct ic *ic, int fd);
int ICflushall(struct ic *ic);
int ICadd(struct ic *ic, int fd, char *item);
void ICprint(FILE *f, struct ic *ic, int data);
#endif
