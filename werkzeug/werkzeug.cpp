#include <iostream>
#include <cstring>
#include <sstream>
#include <anabel/anabel.h>
#include "simple_ops.h"
#include "streamin.h"

using namespace std;
void display_usage_information() {
	cout << "Werkzeug version 1.0" << endl << "Copyright (c) 2012 Piotr Maslanka" << "Part of Anabel time-series database" << endl;
	cout << " -- THIS IS BUT A SIMPLE WRAPPER. WATCH YOUR INPUT!!! -- " << endl;
	cout << "Available types: int32 float int8" << endl;
	cout << " Stream in data from stdin, save to an existing timeseries" << endl;
	cout << " Data is read from stdin in the same format that anabel-werkzeug view outputs" << endl << endl;
	cout << "     anabel-werkzeug streamin <path_to_db> <type>" << endl;
	cout << " Indenting a timeseries"<< endl << endl;
	cout << "     anabel-werkzeug indent <path_to_db>" << endl;
	cout << " Viewing a timeseries" << endl << endl;
	cout << "     anabel-werkzeug view <path_to_db> <timestamp_start> <timestamp_end> <type>" << endl;
	cout << " Appending to a timeseries" << endl << endl;
	cout << "     anabel-werkzeug append <path_to_db> <timestamp> <value> <type>" << endl;
	cout << " Creating a new database" << endl << endl;
	cout << "     anabel-werkzeug create <path_to_db> <record_size>" << endl;
	cout << " Truncating a database" << endl << endl;
	cout << "     anabel-werkzeug truncate <path_to_db>" << endl;

	system("pause");
}

int main(int argc, char* argv[])
{
	if (argc < 3) { display_usage_information(); return 1; }
	if (strcmp(argv[1], "indent")==0) {
		if (argc != 3)
		return indent(argv[2]);
	}

	if (strcmp(argv[1], "streamin")==0) {
		if (argc != 4) { display_usage_information(); return 1; }
		string dtype = argv[3];
		return streamin(argv[2], dtype);
	}

	if (strcmp(argv[1], "append")==0) {
		if (argc != 6) { display_usage_information(); return 1; }
		Anabel::Timestamp timestamp;
		stringstream(argv[3]) >> timestamp;
		string value = argv[4];
		return append(argv[2], timestamp, value, argv[5]);
	}

	if (strcmp(argv[1], "view")==0) {
		if (argc != 6) { display_usage_information(); return 1; }
		Anabel::Timestamp t_from, t_to;
		stringstream(argv[3]) >> t_from;
		stringstream(argv[4]) >> t_to;
		if (t_to < t_from) return 4;
		return view(argv[2], t_from, t_to, argv[5]);
	}

	if (strcmp(argv[1], "create")==0) {
		if (argc != 4) { display_usage_information(); return 1; }
		int record_size;
		stringstream(argv[3]) >> record_size;
		return create(argv[2], record_size);
	}

	if (strcmp(argv[1], "truncate") == 0) {
		if (argc != 3) { display_usage_information(); return 1; }
		return truncate(argv[2]);
	}

	system("pause");
	return 0;
}

/**
Code error list:
	0: Success
	1: Invalid parameter
	2: Database does not exist
	3: Invalid type
	4: Query is wrong
*/
