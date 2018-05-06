#include "odr.h"

#include <iostream>
#include <iterator>

namespace  {

template<class Callable>
auto negate(Callable func) {
	return [func](auto&&...x)->bool { return !func(x...); };
}

}

template<> std::unique_ptr<Body> make_body<clang::RecordDecl>(const clang::RecordDecl* r) {
	return std::make_unique<Record>(r);
}
template<> std::unique_ptr<Body> make_body<clang::FunctionDecl>(const clang::FunctionDecl* r) {
	return std::make_unique<Function>(r);
}

void OdrMap::transform_entries() {
	auto &s = entries_by_line;
	for (auto it = begin(s); it != end(s); ++it) {
		auto ex = s.extract(it);
		PrimaryEntry &emp = ex.value();
		name_to_definition[emp.name].emplace_back(std::move(emp));
	}
}

void OdrMap::report_duplicates() {
	for (auto &p : name_to_definition) {
		auto &name = p.first;
		auto &definitions = p.second;
		auto f = std::adjacent_find(begin(definitions), end(definitions), negate(PrimaryEntry::equal_by_body{}));
		if ( f != end(definitions) ) {
			auto f1 = f;
			std::advance(f1,1);
			std::cout << name << " collision:\n  " << f->location << "\n  " << f1->location << "\n";
		}
	}
}
