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
#include <algorithm>
#include <deque>
#include <boost/utility.hpp>

using namespace boost::filesystem;
using namespace boost::interprocess;
using namespace Anabel::Exceptions;
using namespace std;
using namespace Anabel;

inline Timestamp string_to_timestamp(string str) {
	std::stringstream s(str);
	Timestamp t;
	s >> t;
	return t;
}
inline string timestamp_to_string(Timestamp timestamp) {
	std::stringstream s;
	s << timestamp;
	return s.str();
}

/**
* Scans a directory, returning a vector of timestamp-objects found inside
*/
vector<Timestamp> scan_directory(path directory) {
	vector<path> files_p;
	copy(directory_iterator(directory), directory_iterator(), back_inserter(files_p));
	vector<Timestamp> files;

	for (vector<path>::iterator iter = files_p.begin(); iter != files_p.end(); iter++)
		try {
			files.push_back(string_to_timestamp(iter->filename().string()));
		} catch(...) {}

	return files;
}
/**
* Chooses a timestamp that can be chosen, so it contains needle.
* 
* Haystack must not be empty - other way, it would not make any sense, would it?
* Haystack must contain entry that will follow us to needle.
*/
Timestamp choose(vector<Timestamp> haystack, Timestamp needle) {
	for (unsigned i=0; i<haystack.size(); i++) {
		if (needle <= haystack[i]) return haystack[i];
	}
	throw InternalError("needle not found");
}

Anabel::ReadQuery * Anabel::TimeSeries::get_query(Anabel::Timestamp from, Anabel::Timestamp to) {
	// Sanity checks
	if (this->type == TST_UNUSABLE) throw InvalidInvocation("time series has unusable type");
	if ((this->mode != TSO_READ) && (this->mode != TSO_WRITE)) throw InvalidInvocation("invalid open mode");

	typedef vector<Timestamp> timevector;
	typedef vector<Timestamp>::iterator timevectoriter;

	// Init variables
	path cpath = *this->root_path;
	timevector elements;
	Timestamp choice;
	Anabel::ReadQuery * rq = new ReadQuery();
	rq->files = new deque<path>();
	rq->start = from;
	rq->stop = to;

	// Locate LBA
	cpath = *this->root_path;
	try {
		while (is_directory(cpath)) {
			elements = scan_directory(cpath);
			sort(elements.begin(), elements.end());
			choice = choose(elements, from);
			cpath /= timestamp_to_string(choice);
		}
	} catch (InternalError e) {
		// query empty.
		return rq;
	}

	rq->files->push_back(cpath);

	// Now we will trace thru the filesystem, finding doodz. First up the tree, and then descent.

	while (true) {
		path cpath_parent(cpath.parent_path());
		Timestamp cpath_filename_t = string_to_timestamp(cpath.filename().string());

		elements = scan_directory(cpath_parent);
		sort(elements.begin(), elements.end(), greater<Timestamp>());

		timevectoriter bound = upper_bound(elements.begin(), elements.end(), to, greater<Timestamp>());
		if (bound != elements.end()) {  // we need this check - last element may be the one we are looking for. If it is, we'll trace it later.
			timevector temptrace;
			for (timevectoriter iter = bound; *iter>cpath_filename_t; iter++) temptrace.push_back(*iter);
			for (timevectoriter iter = temptrace.begin(); iter<temptrace.end(); iter++)
				rq->files->push_front(cpath_parent / timestamp_to_string(*iter));
		}

		if (bound==elements.begin()) { // we have to examine the parent directory 
			cpath = cpath.parent_path();
		} else {
			bound--;	// bound is object we should examine now
			cpath = cpath_parent / timestamp_to_string(*bound);
			if (is_regular_file(cpath)) {
				rq->files->push_front(cpath);		// UBA file found
				cout << cpath.string() << " is a regular file" << endl;
				break;
			}
			// this is not a real file. we will use a "fake" file to force the algoritm to work properly with a bound
			cpath /= timestamp_to_string(to);	// kinda faking a file, but algorithm doesn't care
		}
	}

	cout << "[DEBUG] Tracing: " << endl;
	for (deque<path>::iterator iter = rq->files->begin(); iter<rq->files->end(); iter++)
		cout << iter->string() << endl;

	return rq;
}

Anabel::TimeSeries::TimeSeries(std::string rootdirpath, Anabel::TimeSeriesOpenMode open_mode) {
	this->mode = TSO_CLOSED;
	this->type = TST_UNUSABLE;
	// Prepare pathes
	this->root_path = new path(rootdirpath);
	path conf_path(*this->root_path);
	conf_path /= "conf";
	path alock_path(*this->root_path);
	alock_path /= "alock";
	path block_path(*this->root_path);
	block_path /= "block";

	// Sanity-check pathes
	if (!exists(*this->root_path)) throw InvalidRootDirectory("root directory does not exist");
	if (!is_directory(*this->root_path)) throw InvalidRootDirectory("root directory is not a directory");
	if (!exists(conf_path)) throw InvalidRootDirectory("conf file not found");
	if (!exists(alock_path)) throw InvalidRootDirectory("append lock not found");
	if (!exists(block_path)) throw InvalidRootDirectory("rebalance lock not found");

	// Create locks
	this->alock = new boost::interprocess::file_lock(alock_path.string().c_str());
	this->block = new boost::interprocess::file_lock(block_path.string().c_str());
	
	// Do the locking!
	/*
		Now, how do I resolve deadlocks? They may happen.
		During a deadlock, second acquisiton will raise an exception. Then, I will release the locks. 
		And fun begins again. Maybe backoff algorithm can be used by the upper layer? 
	*/
	switch (open_mode) {
		case TSO_APPEND:
			try {
				this->alock->try_lock();
			} catch (boost::interprocess::lock_exception exc) {
				throw TimeSeriesLocked();
			}
			break;
		case TSO_REBALANCE:
			try {
				this->block->try_lock();
			} catch (boost::interprocess::lock_exception exc) {
				throw TimeSeriesLocked();
			}
			break;
		case TSO_READ:
			try {
				this->alock->try_lock_sharable();
			} catch (boost::interprocess::lock_exception exc) {
				throw TimeSeriesLocked();
			}
			try {
				this->block->try_lock_sharable();
			} catch (boost::interprocess::lock_exception exc) {
				this->alock->unlock_sharable();
				throw TimeSeriesLocked();
			}
			break;
		case TSO_WRITE:
			try {
				this->alock->try_lock();
			} catch (boost::interprocess::lock_exception exc) {
				throw TimeSeriesLocked();
			}
			try {
				this->block->try_lock();
			} catch (boost::interprocess::lock_exception exc) {
				this->alock->unlock();
				throw TimeSeriesLocked();
			}
			break;
		case TSO_CLOSED:	// funnt, not undefined though
			return;
			break;
		default:
			throw InvalidInvocation("unknown open_mode");
	}
	this->mode = open_mode;

	// Read type of time series
	std::ifstream conf(conf_path.string());
	unsigned int ftype;
	conf >> ftype;
	conf.close();

	// Verify validity and save to class
	if (ftype >= _TST_GUARD_MAX) ftype = TST_UNUSABLE;
	this->type = (Anabel::TimeSeriesType)ftype;

	// The class is good to go.
}
Anabel::TimeSeries::~TimeSeries() {
	switch (this->mode) {
		case TSO_READ:
			this->alock->unlock_sharable();
			this->block->unlock_sharable();
			break;
		case TSO_WRITE:
			this->alock->unlock();
			this->block->unlock();
			break;
		case TSO_REBALANCE:
			this->block->unlock();
			break;
		case TSO_APPEND:
			this->alock->unlock();
			break;
		case TSO_CLOSED:
			break;
	}
	delete this->alock;
	delete this->block;
	delete this->root_path;
}
