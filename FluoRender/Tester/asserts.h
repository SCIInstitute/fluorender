#pragma once
#include <iostream>

#define ASSERT_EQ(expected, actual) \
	if (expected == actual) \
		std::cout << "SUCCESS: " << expected << " equals " << actual << "." << std::endl; \
	else \
		std::cout << "FAIL: " << expected << " does not equal " << actual << "." << std::endl

#define ASSERT_NEQ(expected, actual) \
	if (expected == actual) \
		std::cout << "FAIL: " << expected << " equals " << actual << "." << std::endl; \
	else \
		std::cout << "SUCCESS: " << expected << " does not equal " << actual << "." << std::endl

#define ASSERT_TRUE(condition) \
	if (condition) \
		std::cout << "SUCCESS." << std::endl; \
	else \
		std::cout <<"FAIL." << std::endl

#define ASSERT_FALSE(condition) \
	if (condition) \
		std::cout << "FAIL." << std::endl; \
	else \
		std::cout <<"SUCCESS." << std::endl
