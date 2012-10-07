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
#include <iostream>
#include <vector>
#include <deque>
#include <boost/interprocess/sync/file_lock.hpp>
#include <boost/filesystem.hpp>
#include "anabel/anabel.h"

namespace Anabel {
	enum TimeSeriesType {
		TST_UNUSABLE,
		TST_INT32,
		_TST_GUARD_MAX,	// don't move - this is in itself an invalid type
	};

	enum TimeSeriesOpenMode {
		TSO_CLOSED,
		TSO_READ,
		TSO_APPEND,
		TSO_REBALANCE,
		TSO_WRITE,
	};

	class ReadQuery {
		friend class TimeSeries;
		private:
			Anabel::Timestamp start;
			Anabel::Timestamp stop;
			std::deque<boost::filesystem::path> * files;
		public:
			~ReadQuery();
	};

	class TimeSeries {
		private:
			boost::interprocess::file_lock * alock;
			boost::interprocess::file_lock * block;
			boost::filesystem::path * root_path;
		public:
			TimeSeriesOpenMode mode; // don't modify from userland
			TimeSeriesType type;     // don't modify from userland
			TimeSeries(std::string rootdirpath);
			~TimeSeries();
			ReadQuery * get_query(Anabel::Timestamp from, Anabel::Timestamp to);
			void open(TimeSeriesOpenMode open_mode);
			void close(void);
	};
};
