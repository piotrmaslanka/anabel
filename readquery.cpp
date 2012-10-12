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
#include <anabel/anabel.h>
void Anabel::ReadQuery::set_desired_cache_size(unsigned elements) { this->desired_cache_size = elements; }
Anabel::ReadQuery::ReadQuery(Timestamp from, Timestamp to, std::vector<boost::filesystem::path> * files, TimeSeriesType type) : from(from), to(to), 
																																files(files), type(type),
																																desired_cache_size(20000) {};
Anabel::ReadQuery::~ReadQuery() {
	if (this->cache_entries > 0) free(this->data_cache);
	delete this->files;
}