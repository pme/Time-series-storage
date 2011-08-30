/*
 * =====================================================================================
 *
 *       Filename:  pr.c
 *
 *    Description:  Pipe reader
 *
 *        Version:  1.0
 *        Created:  2011-07-12 17:55
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Peter Meszaros (pme), hauptadler@gmail.com
 *        Company:  
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
#include <limits.h>
#include <libgen.h>
#include <err.h>
#include <string.h>

int main(int argc, char *argv[])
{
  FILE *f;
  char buf[PIPE_BUF];

  if (argc != 2)
    errx(EXIT_FAILURE, "Usage: %s pipename", basename(argv[0]));

  if ((f = fopen(argv[1], "w+")) == NULL)
    err(EXIT_FAILURE, "open(%s)", argv[1]);

  while (!feof(f) && !ferror(f) && fgets(buf, PIPE_BUF, f)) {
    buf[strlen(buf)-1] = '\0';
    printf("line: >%s<\n", buf);
  }

  fclose(f);
    
  return 0;
}
