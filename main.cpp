#include <iostream>
#include <anabel/anabel.h>
#include <vector>
using namespace std;

int main() {
	Anabel::TimeSeries timeseries("d:\\mirabelka");
	timeseries.open(Anabel::TSO_READ);
	Anabel::ReadQuery * rq = timeseries.get_query(0, 1000);

	system("pause");
}
