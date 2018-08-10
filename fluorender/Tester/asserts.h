#pragma once
#include <iostream>

#define ASSERT_EQ(expected, actual) \
	if (expected == actual) \
		std::cout << expected << " equals " << actual << "." << std::endl; \
	else \
		std::cout << expected << " does not equal " << actual << "." << std::endl

#define ASSERT_TRUE(condition) \
	if (condition) \
		std::cout << "TRUE." << endl; \
	else \
		std::cout <<"FALSE." << endl