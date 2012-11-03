#include <iostream>
#include <cstring>
#include <sstream>
#include <anabel/anabel.h>
#include "simple_ops.h"

using namespace std;
void display_usage_information() {
	cout << "Werkzeug version 1.0" << endl << "Copyright (c) 2012 Piotr Maslanka" << "Part of Anabel time-series database" << endl;
	cout << " -- THIS IS BUT A SIMPLE WRAPPER. WATCH YOUR INPUT!!! -- " << endl;
	cout << "Available types: int32 float int8" << endl;
	cout << " Normalizing two timeseries, performing an operation and store the results:" << endl << endl;
	cout << "     anabel-werkzeug nos <path_to_db_1> <path_to_db_2> <db_1_timestamp_start> <db_1_timestamp_end>";
	cout << "<db_2_timestamp_start> <db_2_timestamp_end> <operation> <type> <output_database_path>" << endl << endl;
	cout << "  Operation is one of: " << endl << "   add - Adds corresponding records" << "   sub - Substracts corresponding records" << endl;
	cout << "  Databases must have matching types. Output will have the same type. "<<endl;
	cout << " Indenting a timeseries"<< endl << endl;
	cout << "     anabel-werkzeug indent <path_to_db>" << endl;
	cout << " Viewing a timeseries" << endl << endl;
	cout << "     anabel-werkzeug view <path_to_db> <timestamp_start> <timestamp_end> <type>" << endl;
	cout << " Appending to a timeseries" << endl << endl;
	cout << "     anabel-werkzeug append <path_to_db> <timestamp> <value> <type>" << endl;
	cout << " Creating a new database" << endl << endl;
	cout << "     anabel-werkzeug create <path_to_db> <record_size>" << endl;

	system("pause");
}

int main(int argc, char* argv[])
{
	if (argc < 3) { display_usage_information(); return 1; }
	if (strcmp(argv[1], "indent")==0) {
		if (argc != 3) { display_usage_information(); return 1; }
		return indent(argv[2]);
	}
	if (strcmp(argv[1], "nos")==0) {
		if (argc != 11) { display_usage_information(); return 1; }
		string db1_path = argv[2];
		string db2_path = argv[3];
		Anabel::Timestamp db1_from, db1_to, db2_from, db2_to;
		stringstream(argv[4]) >> db1_from;
		stringstream(argv[5]) >> db1_to;
		stringstream(argv[6]) >> db2_from;
		stringstream(argv[7]) >> db2_to;
		string operation = argv[8];
		string commontype = argv[9];
		string dbo_path = argv[10];
		if (db1_to < db1_from) return 4;
		if (db2_to < db2_from) return 4;

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
