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
#pragma once
#include <deque>
#include <vector>
#include <fstream>
#include <anabel/anabel.h>

namespace Anabel {
	namespace Internal {

			class DirectoryIterator {
				private:
					std::deque<boost::filesystem::path> state;
				public:
					bool empty;
					boost::filesystem::path next(void);
					DirectoryIterator(std::vector<boost::filesystem::path> * files);
			};

			class IntelligentFileReader: public std::ifstream {
				private:
					long long start_at_ofs;
					long long end_at_ofs;
					int record_size;
					int records_remaining;
					/**
					* Returns index of record containing given value (or closest match)
					*/
					unsigned locate(Anabel::Timestamp time);
				public:
					IntelligentFileReader(boost::filesystem::path path, int record_size);
					void limit_start(Anabel::Timestamp start);			// cannot be called if get_data was already invoked
					void limit_end(Anabel::Timestamp end);				// cannot be called if get_data was already invoked
					unsigned get_data(unsigned records_to_read, void * buffer);
			};

		std::vector<Timestamp> scan_directory(boost::filesystem::path directory);
		std::string timestamp_to_string(Timestamp timestamp);
		Timestamp string_to_timestamp(std::string str);
	};
};