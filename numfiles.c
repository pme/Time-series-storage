/* vim: set ts=2 sw=2:
 * =====================================================================================
 *
 *       Filename:  numfiles.c
 *
 *    Description:  File generator
 *
 *        Version:  1.0
 *        Created:  2012-01-10 15:47
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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <err.h>
#include <libgen.h>

int main(int argc, char *argv[])
{
	int i;
	int n = atoi(argv[1]);

  if (argc != 3)
		errx(EXIT_FAILURE, "Usage: %s numfiles dirname", basename(argv[0]));

	for(i=1; i < n; i++) {
		int fd;
		char filename[1024];

    snprintf(filename, 1024, "%s/%010d", argv[2], i);

		if ((fd = open(filename, O_CREAT, 0644)) == -1)
			err(EXIT_FAILURE, "open(%s)", filename);

		close(fd);

		if (!(i % 1000)) printf("%d files created\n", i);
	}

	return 0;
}
