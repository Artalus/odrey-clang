#pragma once

class DeclarationOnlyClass;

struct CommonClass {
	int with_variable;
	int inline_method() { return -with_variable; }
	double simple_method(float x) const;
	static void and_static_func() { }
};

inline void inline_func() { }
