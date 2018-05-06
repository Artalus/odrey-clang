#include "a.h"
#include <iostream>

namespace {
struct InNamespaceClass {
	float x;
};

int in_namespace_different_function(float x) {
	return int(x);
}

}

class ClassOnlyInA {
	int x;
	double d;
};

inline int violation_func() {
	std::cout << "violation_func in a\n" << in_namespace_different_function(2.5f) << '\n';
	return 1;
}

inline void ok_inline_func() {}

struct IdenticalClass {
	const char ccc[20];
};

struct ViolationClass {
	float f = 1;
	float ff = 2.3f;
	float fff = 4.5678f;
	void fun() {
		std::cout << f << ' ' << ff << ' ' << fff << '\n';
	}
};

using ViolatingAlias = int;
class SubtleViolationClass {
	ViolatingAlias x;
public:
	ViolatingAlias fun() { return x; }
};

void a_func() {
	ViolationClass d;
	std::cout << "ViolationClass in a\n";
	std::cout << d.f << '\n' << d.ff << '\n' << d.fff << '\n';
	d.fun();
}
