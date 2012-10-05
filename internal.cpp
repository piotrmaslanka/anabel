/* 
    This file is part of Anabel

    Anabel is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    Anabel is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Anabel; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include <anabel/internal.h>
#include <ctime>

using namespace boost::filesystem;
using namespace std;
using namespace Anabel;

vector<Timestamp> examine_directory(path directory) {
	vector<path> files_p;

	copy(directory_iterator(directory), directory_iterator(), back_inserter(files_p));

	vector<Timestamp> files;

	Timestamp temp;

	for (vector<path>::iterator iter = files_p.begin(); iter != files_p.end(); iter++) {
		try {
			stringstream sstr(iter->string());
			sstr >> temp;
			files.push_back(temp);
		} catch(...) {}
	}

	sort(files.begin(), files.end(), greater<Timestamp>());

	return files;
}

vector<string> Anabel::Internal::inquire_files(TimeSeries * timeseries, Timestamp from, Timestamp to) {
	vector<string> vec;

	time_t now_t = time(NULL);	// get current UTC time

	return vec;
}