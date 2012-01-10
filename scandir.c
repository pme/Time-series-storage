/* vim: set ts=2 sw=2:
 * =====================================================================================
 *
 *       Filename:  scandir.c
 *
 *    Description:  Directory scanner
 *
 *        Version:  1.0
 *        Created:  2012-01-10 15:50
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

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/time.h>
#include <err.h>
#include <libgen.h>

int min, max;

int filter(const struct dirent *de)
{
	int n = atoi(de->d_name);

  return (n >= min && n <= max) ? 1 : 0;
}

int main(int argc, char *argv[])
{
	int dn;
	int i;
  struct dirent **de = NULL;
	struct timeval fm, to, d;

  if (argc != 4)
		errx(EXIT_FAILURE, "Usage: %s min max dirpath", basename(argv[0]));

  min = atoi(argv[1]);
  max = atoi(argv[2]);

  gettimeofday(&fm, NULL);

  if ((dn = scandir(argv[3], &de, filter, versionsort)) == -1)
		err(EXIT_FAILURE, "scandir(%s)", argv[3]);

  gettimeofday(&to, NULL);

  timersub(&to, &fm, &d);
	fprintf(stderr, "time: %ld.%06ld\n", d.tv_sec, d.tv_usec);

  for (i=0; i<dn; i++) {
		printf("%d - '%s'\n", i, de[i]->d_name);
	}

  free(de);

	return 0;
}
