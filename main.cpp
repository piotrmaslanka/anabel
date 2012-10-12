#include <iostream>
#include <anabel/anabel.h>
#include <vector>
#include <boost/filesystem.hpp>
using namespace std;

int main() {
	Anabel::TimeSeries timeseries("d:\\mirabelka");
	timeseries.open(Anabel::TSO_READ);

	timeseries.get_query(10, 800);

	system("pause");
}
