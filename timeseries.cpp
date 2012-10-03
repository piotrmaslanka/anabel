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
#include "timeseries.h"
#include "exceptions.h"
#include <fstream>
#include <boost/filesystem.hpp>

using namespace boost::filesystem;
using namespace boost::interprocess;
using namespace Anabel::Exceptions;

Anabel::TimeSeries::TimeSeries(std::string rdpath, bool allow_write, bool wait_if_locked) {
	// Sanity checks on the path
	path rootdir(rdpath);

	if (!exists(rdpath)) throw InvalidRootDirectory("Root directory does not exist");
	if (!is_directory(rdpath)) throw InvalidRootDirectory("Root directory not a directory");

	path conffile_path(rootdir);
	conffile_path /= "conf";
	path lockfile_path(rootdir);
	lockfile_path /= "lock";

	if (!exists(conffile_path)) throw InvalidRootDirectory("conf file not found");

	// Attempt to properly lock the file
	try {
		this->flock = new file_lock((char*)(lockfile_path.string().c_str()));
		if (allow_write) {
			if (wait_if_locked) this->flock->lock();
			else this->flock->try_lock();
		} else {
			if (wait_if_locked) this->flock->lock_sharable();
			else this->flock->try_lock_sharable();
		}
	} catch (interprocess_exception exc) {
		throw TimeSeriesLocked();
	}

	this->write_available = allow_write;

	// Read type of time series
	std::ifstream conf(conffile_path.string());
	int ftype;
	conf >> ftype;
	conf.close();

	// Verify validity and save to class
	if ((ftype < TST__GUARD_BOTTOM) || (ftype > TST__GUARD_TOP)) throw UnrecognizedTimeSeriesType();
	this->type = (Anabel::TimeSeriesType)ftype;

	// The class is good to go.
}

Anabel::TimeSeries::~TimeSeries() {
	if (this->write_available)
		this->flock->unlock();
	else
		this->flock->unlock_sharable();
	delete this->flock;
}
