/* vim: set ts=2 sw=2:
 * =====================================================================================
 *
 *       Filename:  ic.h
 *
 *    Description:  ItemCache header -- I/O buffering for time series data
 *
 *        Version:  1.0
 *        Created:  2011-07-31 19:28
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
#include <fcntl.h>
#include <string.h>

#define  IC_ERROR	-1
#define  IC_OK		0

typedef struct ic *IC;
typedef void (*print_t)(FILE *, char *);

struct ic * ICcreate(int nfiles, int itemsize, int nitem, char *path);
ssize_t ICflush(struct ic *ic, int fd);
int ICflushall(struct ic *ic);
int ICdrop(struct ic *ic);
int ICadd(struct ic *ic, int fd, char *item);
void ICprintcache(FILE *f, struct ic *ic, int data, print_t printfunc);
