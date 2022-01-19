#pragma once

#include <iostream>

void trim(std::string &s, const char *t);
void toLowerCase(std::string &s);
#if __cplusplus < 201103L

#include <stdio.h>

#include <string>

std::string to_string(int val);

#endif
