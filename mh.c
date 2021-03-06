/* vim: set ts=2 sw=2:
 * =====================================================================================
 *
 *       Filename:  mh.c
 *
 *    Description:  Mc Pearson hash
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

#define EXP8(X) EXP7(X), EXP7(128+X)
#define EXP7(X) EXP6(X), EXP6( 64+X)
#define EXP6(X) EXP5(X), EXP5( 32+X)
#define EXP5(X) EXP4(X), EXP4( 16+X)
#define EXP4(X) EXP3(X), EXP3(  8+X)
#define EXP3(X) EXP2(X), EXP2(  4+X)
#define EXP2(X) EXP1(X), EXP1(  2+X)
#define EXP1(X) EXP0(X), EXP0(  1+X)
#if 1
#define EXP0(X) ((4*(X)+1)*((X)+1) & 0xff)
#else
#define EXP0(X) ((X) ^ ((X)>>4) + 1) * 19 % 257 - 1
#endif

static unsigned char table[256] = { EXP8(0) };

unsigned hash(const char *key)
{
  /* Mc Pearson's Hash Function */
  const unsigned char *k = (const unsigned char*) key;
  unsigned h = 0, c;
  while ((c = *k++) != 0)
     h = table[h^c];
  return h;
}

