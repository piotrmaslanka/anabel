#include <iostream>
#include <cstring>
#include <sstream>
#include <anabel/anabel.h>
#include "indent.h"

using namespace std;
void display_usage_information() {
	cout << "Werkzeug version 1.0" << endl << "Copyright (c) 2012 Piotr Maslanka" << "Part of Anabel time-series database" << endl;
	cout << " Normalizing two timeseries, performing an operation and storing the results:" << endl << endl;
	cout << "     anabel-werkzeug no <path_to_db_1> <path_to_db_2> <db_1_timestamp_start> <db_1_timestamp_end>";
	cout << "<db_2_timestamp_start> <db_2_timestamp_end> <operation> <type> <output_database_path>" << endl << endl;
	cout << "  Operation is one of: " << endl << "   add - Adds corresponding records" << "   sub - Substracts corresponding records" << endl;
	cout << "  Databases must have matching types. Output will have the same type. "<<endl;
	cout << "  Type is one of: int32 float int8" << endl << endl;
	cout << " Indenting a timeseries"<< endl << endl;
	cout << "     anabel-werkzeug indent <path_to_db>" << endl;
	system("pause");
}

int main(int argc, char* argv[])
{
	if (argc < 3) { display_usage_information(); return 1; }
	if (strcmp(argv[1], "indent")==0) {
		if (argc != 3) { display_usage_information(); return 1; }
		return indent(argv[2]);
	}
	if (strcmp(argv[1], "no")==0) {
		if (argc != 11)  { display_usage_information(); return 1; }
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
	

		
	}


	system("pause");
	return 0;
}

/**
Code error list:
	0: Success
	1: Invalid parameter
	2: Database does not exist
*/
