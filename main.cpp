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

TimeSeries timeseries("d:\\mirabelka");

int main() {
	timeseries.open(TSO_WRITE);

/*	unsigned dupa = 100;
	timeseries.append(9000000, &dupa);*/

	ReadQuery * rq = timeseries.get_query(3999, 4001);
	a_window * lol = (a_window*)malloc(12*100);
	unsigned readed;
	do {
		readed = rq->get_data(100, lol);
		for (int i=0; i<readed; i++) cout << lol[i].timestamp << " " << lol[i].value << endl;
	}
	while (readed != 0);
	free(lol);
	delete rq;

	system("pause");


}
