#pragma once
#include <iostream>

#define ASSERT_EQ(expected, actual) \
	if (expected == actual) \
		std::cout << expected << " equals " << actual << "." << std::endl