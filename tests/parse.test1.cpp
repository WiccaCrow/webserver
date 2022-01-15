#include <string>
#include <iostream>

void skipSpaces(const std::string &line, size_t &pos) {
	for (; line[pos] == ' '; pos++);
}

std::string getData(const std::string &line, size_t &pos) {
	size_t tmp = pos;
	size_t end = pos = line.find(' ', pos);
	return line.substr(tmp, end - tmp);
}

void parseFirstLine(const std::string &line) {
	
	size_t pos = 0;
	
	std::string _method = getData(line, pos);
	skipSpaces(line, pos);
	std::string _path = getData(line, pos);
	skipSpaces(line, pos);
	std::string _protocol = getData(line, pos);
	skipSpaces(line, pos);

	if (line[pos]) //invalid line
		;

	std::cout << "\"" << _method << "\"" << std::endl;
	std::cout << "\"" << _path << "\"" << std::endl;
	std::cout << "\"" << _protocol << "\"" << std::endl;
}

int main() {

	parseFirstLine("GET / HTTP/1.1");
	
	return 0;
}