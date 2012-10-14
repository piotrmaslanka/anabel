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
#include <anabel/stdafx.h>
#include <anabel/anabel.h>

using namespace boost::filesystem;
using namespace boost::interprocess;
using namespace Anabel::Exceptions;
using namespace std;
using namespace Anabel;
using namespace Anabel::Internal;
/**
* Chooses a timestamp that can be chosen, so it contains needle.
* 
* Haystack must not be empty - other way, it would not make any sense, would it?
* Haystack must contain entry that will follow us to needle.
*/
Timestamp choose(vector<Timestamp> haystack, Timestamp needle) {
	for (unsigned i=0; i<haystack.size(); i++) {
		if (haystack[i] <= needle) return haystack[i];
	}
	throw InternalError("needle not found");
}

Anabel::ReadQuery * Anabel::TimeSeries::get_query(Anabel::Timestamp from, Anabel::Timestamp to) {
	// Sanity checks
	if ((this->mode != TSO_READ) && (this->mode != TSO_WRITE)) throw InvalidInvocation("invalid open mode");

	typedef vector<Timestamp> timevector;
	typedef vector<Timestamp>::iterator timevectoriter;

	// Init variables
	path cpath = *this->root_path;
	timevector elements;
	Timestamp choice;
	vector<path> * files = new vector<path>();

	// Locate UBA
	cpath = *this->root_path;
	try {
		while (is_directory(cpath)) {
			elements = scan_directory(cpath);
			sort(elements.begin(), elements.end(), greater<Timestamp>());
			choice = choose(elements, to);
			cpath /= timestamp_to_string(choice);
		}
	} catch (InternalError e) {
		// query empty.
		delete files;
		return new Anabel::ReadQuery(from, to, NULL, this->record_size);
	}

	files->push_back(cpath);

	// Now we will trace thru the filesystem, finding doodz. First up the tree, and then sharp dive towards 'to'

	while (true) {
		path cpath_parent(cpath.parent_path());
		Timestamp cpath_filename_t = string_to_timestamp(cpath.filename().string());

		elements = scan_directory(cpath_parent);
		sort(elements.begin(), elements.end());

		timevectoriter bound = upper_bound(elements.begin(), elements.end(), from);
		vector<path> temp;
		for (timevectoriter iter = bound; *iter<cpath_filename_t; iter++)
			temp.push_back(cpath_parent / timestamp_to_string(*iter));
		for (vector<path>::reverse_iterator iter = temp.rbegin(); iter != temp.rend(); iter++) files->push_back(*iter);

		if (bound==elements.begin()) { // we have to examine the parent directory 
			cpath = cpath.parent_path();
		} else {
			bound--;
			cpath = cpath_parent / timestamp_to_string(*bound);
			if (is_regular_file(cpath)) {
				for (timevectoriter iter = elements.end()-1; iter >= bound; iter--) {
					files->push_back(cpath_parent / timestamp_to_string(*iter));		// LBA file found, we need to finish iterating this directory
					if (iter == elements.begin()) break;
				}
				break;
			}
			// this is not a real file. we will use a "fake" file to force the algoritm to work properly with a bound
			cpath /= timestamp_to_string(from);	// kinda faking a file, but algorithm doesn't care
		}
	}

	return new Anabel::ReadQuery(from, to, files, this->record_size);
}

void Anabel::TimeSeries::open(TimeSeriesOpenMode open_mode) {
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
			throw InvalidInvocation("invalid open_mode");
			break;
		default:
			throw InvalidInvocation("unknown open_mode");
	}

	// Read type of time series, as we don't know it now
	std::ifstream conf((*this->root_path / "record_size").string());
	conf >> this->record_size;
	conf.close();

	this->mode = open_mode;
}

void Anabel::TimeSeries::close(void) {
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
}


Anabel::TimeSeries::TimeSeries(std::string rootdirpath) : mode(TSO_CLOSED), record_size(0) {
	// Prepare pathes
	this->root_path = new path(rootdirpath);
	path rsize_path(*this->root_path);
	rsize_path /= "record_size";
	path alock_path(*this->root_path);
	alock_path /= "alock";
	path block_path(*this->root_path);
	block_path /= "block";

	// Sanity-check pathes
	if (!exists(*this->root_path)) throw InvalidRootDirectory("root directory does not exist");
	if (!is_directory(*this->root_path)) throw InvalidRootDirectory("root directory is not a directory");
	if (!exists(rsize_path)) throw InvalidRootDirectory("rsize_path file not found");
	if (!exists(alock_path)) throw InvalidRootDirectory("append lock not found");
	if (!exists(block_path)) throw InvalidRootDirectory("rebalance lock not found");

	// Create locks
	this->alock = new boost::interprocess::file_lock(alock_path.string().c_str());
	this->block = new boost::interprocess::file_lock(block_path.string().c_str());
}
Anabel::TimeSeries::~TimeSeries() {
	if (this->mode != TSO_CLOSED) this->close();
	delete this->alock;
	delete this->block;
	delete this->root_path;
}
