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
#include <anabel/stdafx.h>
#include <anabel/anabel.h>

using namespace std;
using namespace Anabel;
using namespace boost::filesystem;

Timestamp Anabel::Internal::string_to_timestamp(string str) {
	std::istringstream s(str);
	Timestamp t;
	s >> t;
	if (!s.eof()) throw 1; // we haven't consumed all the string, it means that it was not a number
	return t;
}
string Anabel::Internal::timestamp_to_string(Timestamp timestamp) {
	std::stringstream s;
	s << timestamp;
	return s.str();
}

/**
* Scans a directory, returning a vector of timestamp-objects found inside
*/
vector<Timestamp> Anabel::Internal::scan_directory(boost::filesystem::path directory) {
	vector<path> files_p;
	copy(directory_iterator(directory), directory_iterator(), back_inserter(files_p));
	vector<Timestamp> files;

	for (vector<path>::iterator iter = files_p.begin(); iter != files_p.end(); iter++)
		try {
			files.push_back(string_to_timestamp(iter->filename().string()));
		} catch(...) {}

	return files;
}

Anabel::Internal::IntelligentFileReader::IntelligentFileReader(boost::filesystem::path path, unsigned record_size) : record_size(record_size), start_at_ofs(8) {
	this->open(path.string(), std::ios::binary);
	this->seekg(0, std::ios::end);
	this->end_at_ofs = (unsigned)(this->tellg());		// end of file
	this->seekg(8, std::ios::beg);	// skip the header
	this->records_remaining = (this->end_at_ofs - this->start_at_ofs) / (8 + this->record_size);
	this->total_records = this->records_remaining;

}

unsigned Anabel::Internal::IntelligentFileReader::locate(Anabel::Timestamp time) {
	Anabel::Timestamp temp;
	unsigned imin = 0;
	unsigned imax = this->total_records - 1;
	unsigned imid;
	while (imax >= imin) {
		imid = imin + (imax-imin)/2;
		this->seekg(8+imid*(8+this->record_size), std::ios::beg);
		this->read((char*)(&temp), 8);
		if (temp < time)
			imin = imid + 1;
		else if (temp > time)
			imax = imid - 1;
		else
			return imid;
	}
	return imid;
}
void Anabel::Internal::IntelligentFileReader::prepare_read(void) {
	this->seekg(this->start_at_ofs);
}
void Anabel::Internal::IntelligentFileReader::limit_start(Anabel::Timestamp start) {
	if (this->records_remaining == 0) return;
	this->start_at_ofs = this->locate(start)*(8+this->record_size) + 8;
	this->records_remaining = (this->end_at_ofs - this->start_at_ofs) / (8 + this->record_size);
}
void Anabel::Internal::IntelligentFileReader::limit_end(Anabel::Timestamp stop) {
	if (this->records_remaining == 0) return;
	this->end_at_ofs = (1+this->locate(stop))*(8+this->record_size) + 8;
	this->records_remaining = (this->end_at_ofs - this->start_at_ofs) / (8 + this->record_size);
}
unsigned Anabel::Internal::IntelligentFileReader::get_data(unsigned records_to_read, void * buffer) {
	if (records_to_read > this->records_remaining) records_to_read = records_remaining;

	this->read((char*)buffer, records_to_read*(8 + this->record_size));

	this->records_remaining -= records_to_read;

	return records_to_read;
}

// files are to be passed sorted descending
Anabel::Internal::DirectoryIterator::DirectoryIterator(std::vector<boost::filesystem::path> * files) {
	for (std::vector<boost::filesystem::path>::iterator iter = files->begin(); iter<files->end(); iter++) this->state.push_front(*iter);
	this->empty = (this->state.size() == 0);
	delete files;
}

boost::filesystem::path Anabel::Internal::DirectoryIterator::next(void) {
	if (this->empty) return NULL;
	boost::filesystem::path temp;
	temp = this->state.front();
	this->state.pop_front();
	this->empty = (this->state.size() == 0);
	while (is_directory(temp)) {
		vector<Timestamp> stuff = scan_directory(temp);
		sort(stuff.begin(), stuff.end());
		for (vector<Timestamp>::reverse_iterator iter = stuff.rbegin(); iter != stuff.rend(); iter++) this->state.push_front(temp / timestamp_to_string(*iter));
		temp = this->state.front();
		this->state.pop_front();
		this->empty = (this->state.size() == 0);
	}
	return boost::filesystem::path(temp);
}
