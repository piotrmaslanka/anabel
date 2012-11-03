#include <anabel/anabel.h>
#include <iostream>
#include <sstream>
using namespace Anabel;

int indent(char * db_to_indent) {
	try {
		Anabel::TimeSeries ts(db_to_indent);
		ts.open(TSO_APPEND);
		ts.indent();
		ts.close();
	} catch (Anabel::Exceptions::InvalidRootDirectory) {
		std::cout << "ERROR: Invalid directory" << std::endl;
		return 2;
	}
	return 0;
}

int create(char * db_path, int record_size) {
	Anabel::TimeSeries::create(db_path, record_size);
	return 0;
}

template <class T>
int view_t(Anabel::ReadQuery rq, T referential_type, int record_size) {
	if (sizeof(T) != record_size) {
		std::cout<<"Type doesnt match";
		return 3;		// invalid type
	}

#pragma pack(push, 1)
	struct _e_struct {
		Anabel::Timestamp timestamp;
		T value;
	};
#pragma pack(pop)

	rq.set_desired_cache_size(20000);
	_e_struct * buffer = (_e_struct*)malloc((8+sizeof(T))*20000);

	while (int items_readed = rq.get_data(20000, (void*)buffer))
		for (int i=0; i<items_readed; i++)
			std::cout << buffer[i].timestamp << " " << buffer[i].value << std::endl;

	free((void*)buffer);

	return 0;
}

int append(char * db_path, Anabel::Timestamp timestamp, std::string value, char * commontype) {
	Anabel::TimeSeries * ts;
	try {
		ts = new Anabel::TimeSeries(db_path);
	} catch (Anabel::Exceptions::InvalidRootDirectory) {
		std::cout << "ERROR: Invalid directory" << std::endl;
		return 2;
	}

	ts->open(TSO_APPEND);

	if ((strcmp(commontype, "float")==0) && (ts->record_size != 4)) { delete ts; return 3; }
	if ((strcmp(commontype, "int32")==0) && (ts->record_size != 4)) { delete ts; return 3; }
	if ((strcmp(commontype, "int8")==0) && (ts->record_size != 1)) { delete ts; return 3; }

	char * absurdity_of_life = (char*)malloc(8+ts->record_size);
	*(Anabel::Timestamp*)absurdity_of_life = timestamp;

	if (strcmp(commontype, "float")==0) {
		float f;
		std::stringstream(value) >> f;
		*(float*)(absurdity_of_life+8) = f;
	}
	if (strcmp(commontype, "int32")==0) {
		int f;
		std::stringstream(value) >> f;
		*(int*)(absurdity_of_life+8) = f;
	}
	if (strcmp(commontype, "int8")==0) {
		char f;
		std::stringstream(value) >> f;
		*(char*)(absurdity_of_life+8) = f;
	}

	ts->append((void*)absurdity_of_life);

	free(absurdity_of_life);
	ts->close();
	delete ts;
	return 0;
}

int view(char * db_path, Anabel::Timestamp t_from, Anabel::Timestamp t_to, char * commontype) {
	Anabel::TimeSeries * ts;
	try {
		ts = new Anabel::TimeSeries(db_path);
	} catch (Anabel::Exceptions::InvalidRootDirectory) {
		std::cout << "ERROR: Invalid directory" << std::endl;
		return 2;
	}

	ts->open(TSO_READ);

	Anabel::ReadQuery * rq = ts->get_query(t_from, t_to);	// validity was checked earlier

	int return_value;
	if (strcmp("int32", commontype)==0) return_value = view_t(*rq, (int)0, ts->record_size);
	if (strcmp("int8", commontype)==0) return_value = view_t(*rq, (char)0, ts->record_size);
	if (strcmp("float", commontype)==0) return_value = view_t(*rq, (float)0, ts->record_size);

	delete rq;
	delete ts;
	return return_value;
}