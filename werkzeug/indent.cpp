#include <anabel/anabel.h>
#include <iostream>
using namespace Anabel;

int indent(char * db_to_indent) {
	Anabel::TimeSeries * ts;
	try {
		ts = new Anabel::TimeSeries(db_to_indent);
	} catch (Anabel::Exceptions::InvalidRootDirectory exc) {
		std::cout << "ERROR: Invalid directory" << std::endl;
		return 2;
	}

	ts->open(TSO_APPEND);
	ts->indent();
	ts->close();
	return 0;
}