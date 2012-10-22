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
#include <anabel/stdafx.h>

namespace Anabel {
	namespace Internal {

			class DirectoryIterator {
				private:
					std::deque<boost::filesystem::path> state;
					bool reverse;
				public:
					bool empty;
					boost::filesystem::path next(void);
					DirectoryIterator(std::vector<boost::filesystem::path> files, bool reverse=false);
			};

			class IntelligentFileReader: public std::ifstream {
				private:
					unsigned start_at_ofs;
					unsigned end_at_ofs;
					unsigned record_size;
					unsigned total_records;
					/**
					* Returns index of record containing given value (or closest match)
					*/
					unsigned locate(Anabel::Timestamp time);
				public:
					unsigned records_remaining;
					IntelligentFileReader(boost::filesystem::path path, unsigned record_size);
					/**
					* Seeks to given record. Records are numbered from 0. Throws InvalidInvocation exception if record does not exist.
					* Happily disobeys previous limit_start and limit_end.
					*/
					void seek_record(unsigned record_no);
					void limit_start(Anabel::Timestamp start);		
					void limit_end(Anabel::Timestamp end);			
					void prepare_read(void);	// invoke before using get_data and after limits
					unsigned get_data(unsigned records_to_read, void * buffer);
			};

		std::vector<Timestamp> scan_directory(boost::filesystem::path directory);
		std::string timestamp_to_string(Timestamp timestamp);
		Timestamp string_to_timestamp(std::string str);
	};
};