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
	enum TimeSeriesOpenMode {
		TSO_CLOSED,
		TSO_READ,
		TSO_APPEND,
		TSO_REBALANCE,
		TSO_WRITE,
	};

	class TimeSeries {
		private:
			boost::interprocess::file_lock * alock;
			boost::interprocess::file_lock * block;
			boost::filesystem::path root_path;
		public:
			int record_size;
			TimeSeriesOpenMode mode; // don't modify from userland
			TimeSeries(char * rootdirpath);
			~TimeSeries();
			Anabel::ReadQuery * get_query(Anabel::Timestamp from, Anabel::Timestamp to);
			/**
			* Appends a piece of data to the database.
			* If timestamp is smaller than maximum timestamp present in the database, behaviour is undefined
			*/
			void append(Anabel::Timestamp timestamp, void * value);
			/**
			* Opens the database in given mode
			* Will wait if database is locked.
			*/
			void open(TimeSeriesOpenMode open_mode);
			void close(void);

			/**
			* Creates a new, empty database, whose root directory is specified by rootdirpath.
			* Will create rootdirpath if it doesn't exist
			*/
			static void create(std::string rootdirpath, int record_size);
			/**
			* Clears the database. 
			* Requires database to be open in TSO_WRITE mode
			*/
			void truncate(void);
			/**
			* Returns last entry in given database
			* Returns true if entry was found - else if the DB is empty
			* Buffer is in form:
			*    #pragma pack(1)
			*    struct buffer {
			*         Anabel::Timestamp timestamp;
			*         your_favourite_type value;
			*    };
			**/
			bool get_last(void * buffer);
	};
};
