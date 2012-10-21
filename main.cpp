#include <iostream>
#include <anabel/anabel.h>
#include <vector>
#include <boost/filesystem.hpp>
using namespace std;
using namespace Anabel;

#pragma pack(1)
struct a_window {
	Anabel::Timestamp timestamp;
	unsigned value;
};

TimeSeries timeseries("d:\\test");

int main() {
	timeseries.open(TSO_WRITE);



	ReadQuery * rq = timeseries.get_query(3999, 4001);
	a_window * lol = (a_window*)malloc(12*100);

	lol[0].timestamp = 1234;
	lol[0].value = 1234;

	timeseries.get_last(lol);
	cout << "Last entry: " << lol[0].timestamp << " " << lol[0].value << endl;
/*
	unsigned readed;
	do {
		readed = rq->get_data(100, lol);
		for (int i=0; i<readed; i++) cout << lol[i].timestamp << " " << lol[i].value << endl;
	}
	while (readed != 0);*/
	free(lol);
	delete rq;

	system("pause");


}
