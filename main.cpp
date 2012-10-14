#include <iostream>
#include <anabel/anabel.h>
#include <vector>
#include <boost/filesystem.hpp>
using namespace std;

#pragma pack(1)
struct a_window {
	Anabel::Timestamp timestamp;
	unsigned value;
};

int main() {
	a_window * lol;

	Anabel::TimeSeries timeseries("d:\\mirabelka");
	timeseries.open(Anabel::TSO_READ);

	Anabel::ReadQuery * rq = timeseries.get_query(10, 200);

	lol = (a_window*)malloc(12*100);

	unsigned readed;
	do {
		readed = rq->get_data(100, lol);
		for (int i=0; i<readed; i++) 
			cout << lol[i].timestamp << " " << lol[i].value << endl;
		system("pause");
	}
	while (readed != 0);

}
