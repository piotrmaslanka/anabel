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

void Anabel::TimeSeries::create(char * rootdirpath, int record_size) {
	path rootpath(rootdirpath);
	try {
		remove_all(rootpath);
	} catch(...) {}
	try {
		create_directory(rootpath);
	} catch (...) {}
	ofstream alock((rootpath / "alock").c_str()); alock.close();
	ofstream block((rootpath / "block").c_str()); block.close();
	ofstream rsf((rootpath / "record_size").c_str()); rsf << record_size; rsf.close();
	make_empty_dataset(rootpath / "0");
}

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

Anabel::ReadQuery Anabel::TimeSeries::get_query(Anabel::Timestamp from, Anabel::Timestamp to) throw(Anabel::Exceptions::InvalidInvocation) {
	// Sanity checks
	if ((this->mode != TSO_READ) && (this->mode != TSO_WRITE)) throw InvalidInvocation("invalid open mode");
	if (from>to) throw InvalidInvocation("from>to");

	typedef vector<Timestamp> timevector;
	typedef vector<Timestamp>::iterator timevectoriter;

	// Init variables
	path cpath = this->root_path;
	timevector elements;
	Timestamp choice;
	vector<path> files;

	// Locate UBA
	cpath = this->root_path;
	try {
		while (is_directory(cpath)) {
			elements = scan_directory(cpath);
			sort(elements.begin(), elements.end(), greater<Timestamp>());
			choice = choose(elements, to);
			cpath /= timestamp_to_string(choice);
		}
	} catch (InternalError e) {
		// query empty.
		return Anabel::ReadQuery(from, to, files, this->record_size);
	}

	files.push_back(cpath);
	if (choice <= from) {
		return Anabel::ReadQuery(from, to, files, this->record_size);	// response is a single-file wonder
	}

	cpath = cpath.parent_path();

	// Now we will trace thru the filesystem, finding doodz. First up the tree, and then sharp dive towards 'to'
	Timestamp previous_choice = choice;
	while (true) {
		elements = scan_directory(cpath);
		sort(elements.begin(), elements.end(), std::greater<Timestamp>());
		timevectoriter bound = lower_bound(elements.begin(), elements.end(), from, std::greater<Timestamp>());

		if (bound == elements.end()) {	// we need to ascend
			// index files from bound upwards
			for (timevectoriter start_bound = upper_bound(elements.begin(), elements.end(), to, std::greater<Timestamp>()); start_bound != elements.end(); start_bound++) {
				if (previous_choice == *start_bound) continue;
				previous_choice = *start_bound;
				files.push_back(cpath / timestamp_to_string(*start_bound));
			}
			cpath = cpath.parent_path();
			continue;
		}

		// Descent to "bound"
		for (timevectoriter start_bound = upper_bound(elements.begin(), elements.end(), to, std::greater<Timestamp>()); start_bound != bound; start_bound++) {
			if (previous_choice == *start_bound) continue;
			previous_choice = *start_bound;
			files.push_back(cpath / timestamp_to_string(*start_bound));
		}

		cpath = cpath / timestamp_to_string(*bound);
		if (is_regular_file(cpath)) {
			files.push_back(cpath);
			break;
		}
	}

	return Anabel::ReadQuery(from, to, files, this->record_size);
}

void Anabel::TimeSeries::truncate(void) throw(Anabel::Exceptions::InvalidInvocation) {
	if (this->mode != TSO_WRITE) throw InvalidInvocation("invalid open mode");

	vector<Timestamp> elements = scan_directory(this->root_path);
	for (vector<Timestamp>::iterator iter = elements.begin(); iter != elements.end(); iter++) remove_all(this->root_path / Anabel::Internal::timestamp_to_string(*iter));
	make_empty_dataset(this->root_path / "0");
}

void Anabel::TimeSeries::append(void * value) throw(Anabel::Exceptions::InvalidInvocation) {
	this->get_appending_session().append(value);
}

void Anabel::TimeSeries::open(TimeSeriesOpenMode open_mode) throw(Anabel::Exceptions::InvalidInvocation) {
	/* Locking interaction table (whether two types of locks may be acquired if the other is present):
				READ			WRITE			REBALANCE			APPEND
	READ        YES             NO              NO                  YES
	WRITE       NO              NO              NO                  NO
	REBALANCE   NO              NO              NO                  YES
	APPEND      YES             NO              YES                 NO

	Types of locks acquired (eXclusive, Shareable):
		READ:		S(A)
		WRITE:		X(A)+X(B)
		APPEND:		X(B)
		REBALANCE:	X(A)

	As you may see, deadlocks will not happen
	*/
	switch (open_mode) {
		case TSO_APPEND:
			this->block->lock();
			break;
		case TSO_REBALANCE:
			this->alock->lock();
			break;
		case TSO_READ:
			this->alock->lock_sharable();
			break;
		case TSO_WRITE:
			this->alock->lock();
			this->block->lock();
			break;
		case TSO_CLOSED:	// funnt, not undefined though
			throw InvalidInvocation("invalid open_mode");
			break;
		default:
			throw InvalidInvocation("unknown open_mode");
	}

	// Read type of time series, as we don't know it now
	std::ifstream conf((this->root_path / "record_size").string().c_str());
	conf >> this->record_size;
	conf.close();

	this->mode = open_mode;
}

void Anabel::TimeSeries::indent(void) throw(Anabel::Exceptions::InvalidInvocation) {

	if ((this->mode != TSO_APPEND) && (this->mode != TSO_WRITE)) throw InvalidInvocation("invalid open mode");
	vector<Timestamp> elements = scan_directory(this->root_path);
	sort(elements.begin(), elements.end());
	vector<path> files;
	for (vector<Timestamp>::iterator iter = elements.begin(); iter != elements.end(); iter++)
		files.push_back(this->root_path / timestamp_to_string(*iter));

	DirectoryIterator diter(files, true);

	Anabel::Timestamp * timestamp = (Anabel::Timestamp*)malloc(8+this->record_size);

	while (true) {
		if (diter.empty) return;
		IntelligentFileReader ifr(diter.next(), this->record_size);
		if (ifr.records_remaining > 0) {
			ifr.seek_record(ifr.records_remaining-1);
			ifr.get_data(1,(void*)timestamp);
			make_empty_dataset(this->root_path / timestamp_to_string((*timestamp)+1));
		}
		ifr.close();
	}
}


bool Anabel::TimeSeries::get_last(void * buffer) throw(Anabel::Exceptions::InvalidInvocation) {
	if ((this->mode != TSO_READ) && (this->mode != TSO_WRITE)) throw InvalidInvocation("invalid open mode");
	
	vector<Timestamp> elements = scan_directory(this->root_path);
	sort(elements.begin(), elements.end());
	vector<path> files;
	for (vector<Timestamp>::iterator iter = elements.begin(); iter != elements.end(); iter++)
		files.push_back(this->root_path / timestamp_to_string(*iter));

	DirectoryIterator diter(files, true);

	while (true) {
		if (diter.empty) return false;
		IntelligentFileReader ifr(diter.next(), this->record_size);
		if (ifr.records_remaining > 0) {
			ifr.seek_record(ifr.records_remaining-1);
			ifr.get_data(1,buffer);
			return true;
		}
		ifr.close();
	}
}

void Anabel::TimeSeries::close(void) {
	switch (this->mode) {
		case TSO_READ:
			this->alock->unlock_sharable();
			break;
		case TSO_WRITE:
			this->alock->unlock();
			this->block->unlock();
			break;
		case TSO_REBALANCE:
			this->alock->unlock();
			break;
		case TSO_APPEND:
			this->block->unlock();
			break;
		case TSO_CLOSED:
			break;
	}
	this->mode = TSO_CLOSED;
}


Anabel::TimeSeries::TimeSeries(char * rootdirpath) throw(Anabel::Exceptions::InvalidRootDirectory) : mode(TSO_CLOSED), record_size(0) {
	// Prepare pathes
	this->root_path = rootdirpath;
	path rsize_path(this->root_path);
	rsize_path /= "record_size";
	path alock_path(this->root_path);
	alock_path /= "alock";
	path block_path(this->root_path);
	block_path /= "block";

	// Sanity-check pathes
	if (!exists(this->root_path)) throw InvalidRootDirectory("root directory does not exist");
	if (!is_directory(this->root_path)) throw InvalidRootDirectory("root directory is not a directory");
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
}


Anabel::AppendingSession Anabel::TimeSeries::get_appending_session() throw(Anabel::Exceptions::InvalidInvocation) {
	if ((this->mode != TSO_APPEND) && (this->mode != TSO_WRITE)) throw InvalidInvocation("invalid open mode");

	path path(this->root_path);
	while (!is_regular_file(path)) {
		vector<Timestamp> elements = scan_directory(path);
		path /= timestamp_to_string(*max_element(elements.begin(), elements.end()));
	}

	ofstream * k = new ofstream(path.string().c_str(), std::ios::binary | std::ios::app);
	return AppendingSession(k, this->record_size);
}

Anabel::AppendingSession::AppendingSession(std::ofstream * fhandle, int record_size) : fhandle(fhandle), record_size(record_size) {}

void Anabel::AppendingSession::close() {
	this->fhandle->close();
	delete this->fhandle;
}

Anabel::AppendingSession::~AppendingSession() {
	if (this->fhandle != NULL) this->close();
}

void Anabel::AppendingSession::append(void * buffer) {
	this->append_many(buffer, 1);
}
void Anabel::AppendingSession::append_many(void * buffer, int count) {
	this->fhandle->write((char*)buffer, (8+this->record_size)*count);
}

