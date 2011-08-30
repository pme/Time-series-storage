/*
 * =====================================================================================
 *
 *       Filename:  ic.c
 *
 *    Description:  ItemCache -- I/O buffering for time series data
 *
 *        Version:  1.0
 *        Created:  2011-07-31 19:28
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

struct filebuf {
  int items;
  int fd;
  char *buf;
};


enum errcode {
  IC_OK,
};

struct ic {
  int nfiles;
  int itemsize;
  int nitem;
  char *path;
  enum errcode err;
  struct filebuf fb[];
};


/* Write "n" bytes to a descriptor. */
static ssize_t writen(int fd, const void *vptr, size_t n)
{
	size_t nleft, nwritten;
	const char	*ptr;

	ptr = vptr;	/* can't do pointer arithmetic on void* */
	nleft = n;
	while (nleft > 0) {
		if ((nwritten = write(fd, ptr, nleft)) <= 0)
			return nwritten;		/* error */

		nleft -= nwritten;
		ptr   += nwritten;
	}
	return n;
}

struct ic * ICcreate(int nfiles, int itemsize, int nitem, char *path)
{
  struct ic *ic;  
  int i;
  time_t t;
  struct tm *tm;
  char fname[64+1];
  char p[PATH_MAX+1];

  if ((ic = calloc(1, sizeof(struct ic) + nfiles * sizeof(struct filebuf))) == NULL) return NULL;
  
  ic->nfiles   = nfiles;
  ic->itemsize = itemsize;
  ic->nitem    = nitem;
  ic->path     = path;

  time(&t);
  tm = gmtime(&t);
  strftime(fname, 64, "%F_%T", tm);

  for (i=0; i < ic->nfiles; i++) {
    if ((ic->fb[i].buf = calloc(nitem, itemsize)) == NULL) return NULL;
    snprintf(p, PATH_MAX, "%s/%s_%03d.dat", path, fname, i);
    if ((ic->fb[i].fd = open(p, O_RDWR|O_CREAT|O_TRUNC, 0644)) == -1) return NULL;
  }

  return ic;
}

ssize_t ICflush(struct ic *ic, int fd)
{
  size_t s;

  if ((s = writen(ic->fb[fd].fd, ic->fb[fd].buf, ic->fb[fd].items * ic->itemsize)) == -1) return -1;
  ic->fb[fd].items = 0;

  return s;
}

int ICflushall(struct ic *ic)
{
  int i;

  for(i=0; i<ic->nfiles; i++) {
    if (ICflush(ic, i) == -1) break;
  }
  return i;
}

int ICdrop(struct ic *ic)
{ 
  int i;

  ICflushall(ic);
  for (i=0; i<ic->nfiles; i++) {
    free(ic->fb[i].buf);
    close(ic->fb[i].fd);
  }
  free(ic);

  return 0;
}

int ICadd(struct ic *ic, int fd, char *item)
{
  memcpy(ic->fb[fd].buf + ic->fb[fd].items, item, ic->itemsize);
  ic->fb[fd].items++;
  
  if (ic->fb[fd].items == ic->nitem) ICflush(ic, fd);
  return 0;
}
