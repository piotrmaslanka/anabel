#include <iostream>
#include <anabel/anabel.h>
#include <ctime>

using namespace Anabel;

int main() {

	time_t now_t = time(NULL);

	std::cout << "Obecne godziny: " << (now_t % 86400)/3600 << std::endl;

	system("pause");

	TimeSeries * ts = new TimeSeries("d:/mirabelka", true);
	delete ts;

	system("pause");

}
