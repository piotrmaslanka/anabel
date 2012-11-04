#include <anabel/anabel.h>
using namespace Anabel;
using namespace std;

template <class T>
int normalize_aggregate_store_t(ReadQuery rq1, ReadQuery rq2, TimeSeries output, char * operation, T referential_type) {
}


int normalize_aggregate_store(char * db_path1, char * db_path2, Anabel::Timestamp db1_from, Anabel::Timestamp db1_to, Anabel::Timestamp db2_from,
							  Anabel::Timestamp db2_to, char * operation, char * commontype, char * output_db) {
	auto_ptr<TimeSeries> db1, db2, dbo;

	// Open the databases
	try {
		db1 = auto_ptr<TimeSeries>(new TimeSeries(db_path1));
		db2 = auto_ptr<TimeSeries>(new TimeSeries(db_path2));
		dbo = auto_ptr<TimeSeries>(new TimeSeries(output_db));
	} catch (Exceptions::InvalidRootDirectory) {
		return 1;
	}
	
	if (strcmp(commontype, "int8")==0) {
		if (db1->record_size != 1) return 3;
		if (db2->record_size != 1) return 3;
		if (dbo->record_size != 1) return 3;
	}

	if ((strcmp(commontype, "int32")==0) && (strcmp(commontype, "float")==0)) {
		if (db1->record_size != 4) return 3;
		if (db2->record_size != 4) return 3;
		if (dbo->record_size != 4) return 3;
	}

	db1->open(TSO_READ);
	db2->open(TSO_READ);
	dbo->open(TSO_APPEND);

	ReadQuery rdb1 = db1->get_query(db1_from, db1_to);
	ReadQuery rdb2 = db1->get_query(db2_from, db2_to);

	if (strcmp(commontype, "int8")==0) return normalize_aggregate_store_t(rdb1, rdb2, *dbo, operation, (char)0);
	if (strcmp(commontype, "float")==0) return normalize_aggregate_store_t(rdb1, rdb2, *dbo, operation, (float)0);
	if (strcmp(commontype, "int32")==0) return normalize_aggregate_store_t(rdb1, rdb2, *dbo, operation, (int)0);
}