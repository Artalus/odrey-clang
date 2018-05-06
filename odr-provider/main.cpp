#include <iostream>

#include "a.h"
#include "b.h"
#include "common.h"

double CommonClass::simple_method(float x) const { return 3.5*double(x); }

inline int violation_func() {
	std::cout << "also violation_func in main!\n";
	return 1;
}

int main()
{
	a_func();
	b_func();
	return 0;
}
