#!/usr/bin/perl
# vim: set ts=2 sw=2:
# =====================================================================================
#
#       Filename:  genid.pl
#
#    Description:  Identifier generator for test purpouses.
#
#        Version:  1.0
#        Created:  2011-10-30 00:27
#       Revision:  none
#       Compiler:  gcc
#
#         Author:  Peter Meszaros (pme), hauptadler@gmail.com
#        Company:  Infalk Co.
#
# =====================================================================================
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
# =====================================================================================
#

use Getopt::Std;

sub genid
{
	my $l = shift;

	my @chars = ('a'..'z','A'..'Z','0'..'9','_');
	my $rstr;

	foreach (1..$l) {
		$rstr .= $chars[rand @chars];
	}

	return "$rstr\n";
}

sub usage
{
	print STDERR "Usage: $0 -l length -n idnum\n";
	exit -1;
}

getopts('hn:l:', \%opts) or usage();

usage() if $opts{h};
usage() unless $opts{n} && $opts{l};

for($i=0; $i<$opts{n}; $i++) {
  print genid($opts{l});
}

exit 0;
