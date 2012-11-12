#include <string>
#include <iostream>
#include <anabel/anabel.h>

using namespace Anabel;
using namespace std;

/**
	does anabel-werkzeug view output trailing non-data in the last line? 
	neither should you. Last line should either have valid data and end in EOF, or contain just EOF.
*/

template<class T>
int streamin_t(Anabel::AppendingSession * as, T referential_type) {
	char * opbuf = (char*)malloc(8+sizeof(T));
	Timestamp * pt = (Timestamp*)(opbuf);
	T * pv = (T*)(opbuf+8);

	while (!cin.eof()) {
		cin >> *pt >> *pv;
		as->append(opbuf);
	}

	free((void*)opbuf);
	delete as;
	return 0;
}



int streamin(char * path, std::string type) {
	try {
		TimeSeries ts(path);
		ts.open(TSO_APPEND);

		AppendingSession * as = ts.get_appending_session();	// streamin_t has to free this
		if (type == "int8") return streamin_t(as, (char)0);
		if (type == "int32") return streamin_t(as, (int)0);
		if (type == "float") return streamin_t(as, (float)0);
	} catch (Exceptions::InvalidRootDirectory) {
		return 2;
	}
	return 0;
}
