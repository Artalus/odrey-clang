#include <iostream>

#include "a.h"
#include "b.h"
#include "common.h"

double CommonClass::simple_method(float x) const { return 3.5*double(x); }

inline int violation_func() {
	std::cout << "also violation_func in main!\n";
	return 1;
}

struct {
	int x;
} ok_anonymous_struct;
struct {
	int x;
} ok_another_anonymous_struct;

int main()
{
	a_func();
	b_func();
	return 0;
}
