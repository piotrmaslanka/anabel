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

using namespace boost::filesystem;
using namespace boost::interprocess;
using namespace Anabel::Exceptions;

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
	this->alock = new boost::interprocess::file_lock(alock_path.string());
	this->block = new boost::interprocess::file_lock(block_path.string());
	
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
	if (ftype >= _TST_GUARD_MAX) throw UnrecognizedTimeSeriesType(ftype);
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
