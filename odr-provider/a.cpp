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

namespace a {
    struct SameNameInDifferentNamespace {
		int x;
	};
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

using ViolatingAlias = float;
class SubtleViolationClass {
public:
	ViolatingAlias x;
	void fun() { std::cout << x << '\n'; }
};

void a_func() {
	SubtleViolationClass s;
	s.x = 88.8f;
	s.fun();

	ViolationClass d;
	std::cout << "ViolationClass in a\n";
	std::cout << d.f << '\n' << d.ff << '\n' << d.fff << '\n';
	d.fun();
}
