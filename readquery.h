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
#include <anabel/timeseries.h>
#include <boost/filesystem.hpp>

namespace Anabel {	
	class ReadQuery {
		friend class TimeSeries;
		private:
			Anabel::Timestamp from, to;
			TimeSeriesType type;
			std::vector<boost::filesystem::path> * files;
			void * data_cache;
			unsigned cache_entries;
			unsigned desired_cache_size;
			ReadQuery(Anabel::Timestamp from, Anabel::Timestamp to, std::vector<boost::filesystem::path> * files, Anabel::TimeSeriesType type);
		public:
			void set_desired_cache_size(unsigned elements);
			unsigned get_data(unsigned count, void * buffer);
			~ReadQuery();

	};
};