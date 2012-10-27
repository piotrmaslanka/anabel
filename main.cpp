#include <iostream>
#include <anabel/anabel.h>
#include <vector>
#include <anabel/py-interface.h>
#include <boost/filesystem.hpp>
using namespace std;
using namespace Anabel;

#pragma pack(push,1)
struct a_window {
	Anabel::Timestamp timestamp;
	float value;
};
#pragma pack(pop)

int main() {
	TimeSeries ts("d:/lol");
	ts.open(TSO_READ);

	ReadQuery * rq = ts.get_query(1351073520, 1351371011);

	a_window a;
	while(true) {
		int records_readed = rq->get_data(1, &a);
		if (records_readed == 0) break;
		cout << a.timestamp << " " << a.value << endl;;
		system("pause");
	}

	system("pause");
}
