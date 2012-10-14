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

bool check_range(Timestamp start, Timestamp stop) {
	ReadQuery * rq = timeseries.get_query(start, stop);
	a_window * lol = (a_window*)malloc(12*100);
	unsigned ts_start = start;
	unsigned readed;
	do {
		readed = rq->get_data(100, lol);
		for (int i=0; i<readed; i++)
			if (lol[i].timestamp == ts_start) ts_start++;
			else { 
				cout << "While processing " << start << ":" << stop << endl;
				cout << "Mismatch, expected " << ts_start << " got " << lol[i].timestamp << endl;
				return false;
			}
	}
	while (readed != 0);
	free(lol);
	delete rq;
	return true;
}

int main() {
	timeseries.open(TSO_READ);

	for (int start=0; start<2000; start+=50) {
		cout << "Step " << start << endl;
		for (int stop=start+1; stop<2020; stop+=20)
			if (!check_range(start, stop)) {
				system("pause");
			}
	}
	system("pause");


}
