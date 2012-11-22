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
#include <anabel/anabel.h>
#include <anabel/fsobjects.h>

namespace Anabel {	
	/**
	Class that represents a query made against a particular Anabel data set.
	Refer to Anabel::TimeSeries foreword on what Buffer Format is.
	*/

	class BigDataBlock {
		friend class ReadQuery;
		public:
			void * buffer;
			size_t entries_readed;
			size_t buffer_length;
			BigDataBlock();
			~BigDataBlock();
			/**
			When interfacing with automatically memory-managed systems, calling the destructors might
			be seriously deferred. Unfortunately, BDB's hold a lot of memory in it's buffers, so it 
			may be preferable to have a call that safely frees that memory.
			It is safe to call invalidate(), and then have the object destructor called
			*/
			void invalidate();
	};

	class ReadQuery {
		friend class TimeSeries;
		private:
			Anabel::Timestamp from, to;
			int record_size;
			Anabel::Internal::DirectoryIterator * diter;
			Anabel::Internal::IntelligentFileReader * opened_file;			
			void * data_cache;
			void * cache_ofs;
			size_t available_cache_entries;
			size_t desired_cache_size;
			bool first_readed;
			ReadQuery(Anabel::Timestamp from, Anabel::Timestamp to, std::vector<boost::filesystem::path> files, int record_size);
			/**
			Load desired_cache_size from cache, if it's empty.
			If there are no more files to process, do nothing.
			*/
			void prime_cache(void);
			/**
			Low-level data reader.
			Extracts data from input. Returns zero on no more data. Does nothing with cache
			*/
			size_t ll_get_data(size_t count, void * buffer);
		public:
			/**
			Returns next piece of data. buffer data will be in Buffer Format. buffer must have room to accomodate at least count records
			*/
			size_t get_data(size_t count, void * buffer);
			/**
			Sets internal buffer size. Useful if you have loads of records in a single file, for performance reasons.
			In everyday programming, you don't need to use it. Default value is 20000.
			*/
			void set_desired_cache_size(size_t elements);
			/**
			* Returns current cache size in elements
			*/
			size_t get_cache_size();
			/**
			* Reads in all data, in cache-sized chunks.
			* WARNING!
			*   If you have invoked this, you will realize that Anabel has no way of knowing in advance how much memory
			*   will be needed, so it may need to assemble the data from small chunks.
			*   If you are reading in copious amounts of data, this may cause your program to fail.
			*   YOU HAVE BEEN WARNED.
			*/
			BigDataBlock * read_everything();
			~ReadQuery();
	};
};