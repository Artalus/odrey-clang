#include "a.h"
#include "b.h"
#include "common.h"

double CommonClass::simple_method(float x) const { return 3.5*double(x); }

int main()
{
	a_func();
	b_func();
	return 0;
}
