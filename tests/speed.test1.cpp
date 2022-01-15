#include <vector>
#include <list>
#include <cstring>
#include <chrono>
#include <iostream>
#include <poll.h>

// 2 3 4 -1 6 7 8


// vector, 1000 total time: 0.00000326
// list,   1000 total time: 0.00001612

// vector, 100  total time: 0.00000070
// list,   100  total time: 0.00000196

int main(int ac, char **av) {
	
	if (ac != 2)
		return 1;

	const size_t size = atoi(av[1]);
	// std::list<struct pollfd> l(size, (struct pollfd) { 2, POLLIN, 0 });
	std::vector<struct pollfd> v(size, (struct pollfd) { 2, POLLIN, 0 });

	auto start_time = std::chrono::system_clock::now();

	//auto it = l.begin();
	//auto end = l.end();
	//for (; it != end; it++);
	for (size_t i = 0; i < size; i++);

	auto end_time = std::chrono::system_clock::now();
	std::chrono::duration<double> diff = end_time - start_time;

	printf("total time: %.8f\n", diff.count());
	//std::cout << "total time: " << diff.count() << std::endl;
	
	return 0;
}