#include "b.h"
#include <iostream>

namespace {
struct InNamespaceClass {
	int x;
};

int in_namespace_different_function(int x) {
	return -x;
}

}

int func_only_in_b() { return 1; }

inline int violation_func() {
	std::cout << "another violation_func in b\n" << in_namespace_different_function(11) << '\n';
	return 1;
}

inline void ok_inline_func() {}

struct IdenticalClass {
	const char ccc[20];
};

struct ViolationClass {
	short f = 1;
	short ff = 23;
	short fff = 4567;
	void fun() {
		std::cout << f << ' ' << ff << ' ' << fff << '\n';
	}
};

using ViolatingAlias = std::string;
class SubtleViolationClass {
	ViolatingAlias x;
public:
	ViolatingAlias fun() { return x; }
};

void b_func() {
	ViolationClass d;
	std::cout << "another ViolationClass in b\n";
	std::cout << d.f << '\n' << d.ff << '\n' << d.fff << '\n';
	d.fun();
}
