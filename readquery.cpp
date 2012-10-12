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


void Anabel::ReadQuery::prime_cache(void) {
	if (data_cache != NULL) return;		// cache needs to be null
	if (opened_file == NULL) {
		if (this->files->size() == 0) return;		// nothing to do
		// open new file
		this->opened_file = new IntelligentFileReader(this->files->back(), Anabel::TYPE_SIZE[this->type]);
		this->files->pop_back();

	}


}

unsigned Anabel::ReadQuery::get_data(unsigned count, void * buffer) {

}

Anabel::ReadQuery::IntelligentFileReader::IntelligentFileReader(boost::filesystem::path path, int record_size) : record_size(record_size), start_at_ofs(8) {
	this->open(path.string(), std::ios::in);
	this->seekg(0, std::ios::end);
	this->end_at_ofs = (long long)(this->tellg())-record_size;		// last valid data pointer
	this->seekg(8, std::ios::beg);	// skip the header

}

void Anabel::ReadQuery::IntelligentFileReader::limit_start(Anabel::Timestamp start) {}
void Anabel::ReadQuery::IntelligentFileReader::limit_end(Anabel::Timestamp stop) {}
unsigned Anabel::ReadQuery::IntelligentFileReader::get_data(unsigned bytes_to_read, void * buffer) {};

Anabel::ReadQuery::ReadQuery(Timestamp from, Timestamp to, std::vector<boost::filesystem::path> * files, TimeSeriesType type) : from(from), to(to), 
																																files(files), type(type),
																																first_readed(true),
																																opened_file(NULL), data_cache(NULL),																																
																																desired_cache_size(20000) {
	std::reverse(this->files->begin(), this->files->end());
}
Anabel::ReadQuery::~ReadQuery() {
	if (this->cache_entries > 0) free(this->data_cache);
	delete this->files;
}