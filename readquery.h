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
	Class that represents a query made against a particular Anabel data set
	*/
	class ReadQuery {
		friend class TimeSeries;
		private:
			Anabel::Timestamp from, to;
			int record_size;
			Anabel::Internal::DirectoryIterator * diter;
			Anabel::Internal::IntelligentFileReader * opened_file;			
			void * data_cache;
			void * cache_ofs;
			unsigned available_cache_entries;
			unsigned desired_cache_size;
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
			unsigned ll_get_data(unsigned count, void * buffer);
		public:
			unsigned get_data(unsigned count, void * buffer);
			void set_desired_cache_size(unsigned elements);
			~ReadQuery();

	};
};