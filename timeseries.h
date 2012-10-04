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
#include <iostream>
#include <boost/interprocess/sync/file_lock.hpp>
#include <boost/filesystem.hpp>

namespace Anabel {

	enum TimeSeriesType {
		TST__GUARD_BOTTOM,		// don't move, this is a guard to check for surefire invalid timeseries directories

		TST_INT32,

		TST__GUARD_TOP,			// don't move, this is a guard to check for surefire invalid timeseries directories
	};

	class TimeSeries {
		private:
			bool write_available;
			boost::interprocess::file_lock * flock;
			boost::filesystem::path * root_path;
		public:
			TimeSeriesType type;
			TimeSeries(std::string rdpath, bool allow_write=false, bool wait_if_locked=false);
			~TimeSeries();
	};
};
