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

void Anabel::Internal::make_empty_dataset(boost::filesystem::path path) {
	ofstream ofs(path.c_str(), std::ios::binary);
	ofs.write("ANABEL\x00\x00", 8);
	ofs.close();
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

Anabel::Internal::IntelligentFileReader::IntelligentFileReader(boost::filesystem::path path, size_t record_size) : record_size(record_size), start_at_ofs(8), start_at_record(0) {
	this->open(path.string().c_str(), std::ios::binary);
	this->seekg(0, std::ios::end);
	this->end_at_ofs = (unsigned)(this->tellg());		// end of file
	if (this->end_at_ofs < 8) {
		// !!!!
		// This file is invalid.
		this->records_remaining = 0;
		this->total_records = 0;
		return;
	}
	this->seekg(8, std::ios::beg);	// skip the header
	this->records_remaining = (this->end_at_ofs - this->start_at_ofs) / (8 + this->record_size);
	this->total_records = this->records_remaining;

}
size_t Anabel::Internal::IntelligentFileReader::locate(Anabel::Timestamp time, bool is_start) {
	Anabel::Timestamp temp;
	size_t imin = 0;
	size_t imax = this->total_records - 1;
	size_t amax = imax;
	size_t imid;
	
	while (imax >= imin) {
		imid = imin + ((imax - imin) / 2);
		this->seekg(8+imid*(8+this->record_size), std::ios::beg);
		this->read((char*)(&temp), 8);
		if (temp == time) return imid;
		if (temp < time) {
			if (imid == amax)
				// range exhausted.
				if (is_start) throw (int)1;	// start past last record? nonsense;
				else return amax;

			imin = imid + 1;
		} else {
			if (imid == 0) // range exhausted.
				if (is_start) return 0;
				else throw (int)1;	// stop before first record? nonsense.

			imax = imid - 1;
		}
	}

	// not found and in range. Attempt to interpolate
	if (is_start) {
		if (temp < time) return imid+1;
		else return imid;
	} else {
		if (temp < time) return imid;
		else return imid-1;
	}

	// We shouldn't ever get here!
	throw Anabel::Exceptions::InternalError("Program should not fail like this! Report this error to the developer.");
}
void Anabel::Internal::IntelligentFileReader::prepare_read(void) {
	this->seekg(this->start_at_ofs);
}
void Anabel::Internal::IntelligentFileReader::limit_start(Anabel::Timestamp start) {
	if (this->records_remaining == 0) return;
	try {
		this->start_at_record = this->locate(start, true);
	} catch (int) {
		this->records_remaining = 0;	// data not found here
		return;
	}
	this->start_at_ofs = this->start_at_record*(8+this->record_size) + 8;
	this->records_remaining = (this->end_at_ofs - this->start_at_ofs) / (8 + this->record_size);
}
void Anabel::Internal::IntelligentFileReader::limit_end(Anabel::Timestamp stop) {
	if (this->records_remaining == 0) return;
	size_t x;
	try {
		x = this->locate(stop, false);
	} catch (int) {
		this->records_remaining = 0;
		return;
	}

	this->end_at_ofs = (1+x)*(8+this->record_size) + 8;
	this->records_remaining = (this->end_at_ofs - this->start_at_ofs) / (8 + this->record_size);
}
unsigned Anabel::Internal::IntelligentFileReader::get_data(size_t records_to_read, void * buffer) {
	if (records_to_read > this->records_remaining) records_to_read = records_remaining;
	this->read((char*)buffer, records_to_read*(8 + this->record_size));
	this->records_remaining -= records_to_read;
	return records_to_read;
}
void Anabel::Internal::IntelligentFileReader::seek_record(size_t record_no) {
	if (record_no >= (this->total_records)) throw Anabel::Exceptions::InternalError("Cannot seek to record that does not exist");
	this->seekg(8+(record_no*(8+this->record_size)));
	this->records_remaining = this->total_records - record_no;
}

// files are to be passed sorted descending
Anabel::Internal::DirectoryIterator::DirectoryIterator(std::vector<boost::filesystem::path> files, bool reverse) :reverse(reverse) {
	for (std::vector<boost::filesystem::path>::iterator iter = files.begin(); iter!=files.end(); iter++) this->state.push_front(*iter);
	this->empty = (this->state.size() == 0);
}

boost::filesystem::path Anabel::Internal::DirectoryIterator::next(void) {
	if (this->empty) throw Anabel::Exceptions::InternalError("Empty. It is your duty to ensure it is not before calling next()!");
	boost::filesystem::path temp;
	temp = this->state.front();
	this->state.pop_front();
	this->empty = (this->state.size() == 0);
	while (is_directory(temp)) {
		vector<Timestamp> stuff = scan_directory(temp);
		if (this->reverse)
			sort(stuff.begin(), stuff.end(), std::greater<Timestamp>());
		else
			sort(stuff.begin(), stuff.end());
		for (vector<Timestamp>::reverse_iterator iter = stuff.rbegin(); iter != stuff.rend(); iter++) this->state.push_front(temp / timestamp_to_string(*iter));
		temp = this->state.front();
		this->state.pop_front();
		this->empty = (this->state.size() == 0);
	}
	return temp;
}
