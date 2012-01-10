#!/bin/sh
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

#./rd $1 -i 00Jigqdb0KzK -f "2011-11-06 00:19:00" -t "2011-11-07 00:20:00"
#./rd $1 -i 00Jigqdb0KzK -f "2011-12-11 19:30:00" -t "2011-12-11 20:00:00"
echo "00Jigqdb0KzK|2012-01-10 11:30:00|2012-01-10 12:00:00" | ./rd $1
