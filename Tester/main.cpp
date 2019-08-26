#include <iostream>
#include "tests.hpp"

int main()
{
   ObjectTest();
   std::cout << "Object Test 1 now finished, running object test 2..." << std::endl;
   ObjectTest2();
   std::cout << "Object Test 2 now finished, running object test 3..." << std::endl;
   ObjectTest3();
    return 0;
}
