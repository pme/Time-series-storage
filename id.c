/* vim: set ts=2 sw=2:
 * =====================================================================================
 *
 *       Filename:  id.c
 *
 *    Description:  Inetd handler
 *
 *        Version:  1.0
 *        Created:  2011-12-26 20:11
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
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <libgen.h>
#include <err.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main(int argc, char *argv[])
{
	char line[1024];
	int fd;
	ssize_t s;

  if (argc != 2)
		errx(EXIT_FAILURE, "Usage: %s <pipe-path>", basename(argv[0]));

  if ((fd = open(argv[1], O_RDWR, 0644)) == -1)
		err(EXIT_FAILURE, "open(%s) failed", argv[1]);

	while ((fgets(line, 1024, stdin)) != NULL) {
		int l = strlen(line);
		s = write(fd, line, l);
		if (s == -1)
		  err(EXIT_FAILURE, "write(%s) failed", argv[1]);
		else if (s != l)
		  errx(EXIT_FAILURE, "write(%s) partially failed", argv[1]);
	}

  close(fd);

	return 0;
}
