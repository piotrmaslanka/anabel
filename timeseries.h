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

	class AppendingSession {
		friend class TimeSeries;
		private:
			std::ofstream * fhandle;
			int record_size;
			AppendingSession(boost::filesystem::path path, int rec_size);
		public:
			/**
			Closes the appending session. After that, no more data can be appended
			*/
			void close();
			/**
			Append one item of data. buffer is in Buffer Format
			If timestamp is smaller than maximum timestamp present in the database, behaviour is undefined
			*/
			void append(void * buffer);
			/**
			Append many records. buffer is in Buffer Format, and is array of at least count items.
			Data must be ordered ascending, and if the smallest timestamp is smaller than maximum timestamp present in the database, behaviour is undefined
			*/
			void append_many(void * buffer, int count);
			~AppendingSession();
	};

	/**
	Main class representing a timeseries.
	
	FOREWORD
	Buffer Format is an array of 1 or more entries in format:
	   #pragma pack(1)
	   struct buffer {
	        Anabel::Timestamp timestamp;
	        your_favourite_type value;
	   };
	
	Learn to love it, you will be passing a lot of it in and out.
	*/
	class TimeSeries {
		private:
			boost::interprocess::file_lock * alock;
			boost::interprocess::file_lock * block;
			boost::filesystem::path root_path;
		public:
			/** 
			Size of value, in bytes
			*/
			int record_size;
			TimeSeriesOpenMode mode;
			TimeSeries(char * rootdirpath) throw(Anabel::Exceptions::InvalidRootDirectory);
			~TimeSeries();
			/**
			Creates and returns a new appending session. Timeseries must be open in TSO_WRITE or TSO_APPEND
			*/
			Anabel::AppendingSession * get_appending_session() throw(Anabel::Exceptions::InvalidInvocation);
			/**
			Returns a new ReadQuery instance representing value of query made. Timeseries must be open in TSO_WRITE or TSO_READ
			*/
			Anabel::ReadQuery * get_query(Anabel::Timestamp from, Anabel::Timestamp to) throw(Anabel::Exceptions::InvalidInvocation);
			/**
			Appends a single piece of data to the database. Data must be in Buffer Format.
			If timestamp is smaller than maximum timestamp present in the database, behaviour is undefined
			
			*/
			void append(void * value) throw(Anabel::Exceptions::InvalidInvocation);
			/**
			Opens the database in given mode
			Will wait if database is locked.
			*/
			void open(TimeSeriesOpenMode open_mode) throw(Anabel::Exceptions::InvalidInvocation);
			void close(void);

			/**
			Creates a new, empty database, whose root directory is specified by rootdirpath.
			Will create rootdirpath if it doesn't exist. Will wipe existing stuff at rootdirpath if it exists.
			*/
			static void create(char * rootdirpath, int record_size);
			/**
			Clears the database. 
			Requires database to be open in TSO_WRITE mode
			*/
			void truncate(void) throw(Anabel::Exceptions::InvalidInvocation);

			/**
			Creates a new, empty data set in time series' root directory
			Requires the database to be open in TSO_WRITE or TSO_APPEND mode.
			Will return with nothing if current highest-numbered dataset is empty
			*/
			void indent(void) throw(Anabel::Exceptions::InvalidInvocation);
			/**
			Returns last entry in given database
			Returns true if entry was found - else if the DB is empty
			Buffer is in Buffer Format.
			**/
			bool get_last(void * buffer) throw(Anabel::Exceptions::InvalidInvocation);
	};
};
