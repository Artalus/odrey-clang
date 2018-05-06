#include "odr.h"

#include <iterator>
#include <sstream>

namespace  {

template<class Callable>
auto negate(Callable func) {
	return [func](auto&&...x)->bool { return !func(x...); };
}

void report(llvm::raw_ostream &target, bool silence_multiple, const std::string &name, std::list<PrimaryEntry> &definitions) {
	constexpr unsigned INDENT = sizeof("ODR-1337:");
	auto start = begin(definitions);
	auto &first = *start;
	std::advance(start,1);
	auto equal_end = std::partition(start, end(definitions),
	    [&first](const PrimaryEntry &e) { return !e.equal_by_body_text(first); });
	using C = llvm::raw_ostream::Colors;
	if (equal_end != start) {
		target.changeColor(C::RED, true) << "ODR-1337: ";
		target.changeColor(C::CYAN, true) << name;
		target.changeColor(C::WHITE, true) << " has varying definitions:\n";
		for (auto i = begin(definitions); i != equal_end; ++i)
			target.indent(INDENT).changeColor(C::WHITE) << i->location << "\n";
	} else if (!silence_multiple && definitions.size() > 1) {
		target.changeColor(C::MAGENTA, true) << "ODR-1336: ";
		target.changeColor(C::CYAN, true) << name;
		target.changeColor(C::WHITE, true) << " has multiple definitions, consider unifying those in a single header:\n";
		for (auto &d : definitions)
			target.indent(INDENT).changeColor(C::WHITE) << d.location << "\n";
	}
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

void OdrMap::report_duplicates(llvm::raw_ostream &target, bool silence_multiple) {
	for (auto &p : name_to_definition) {
		report(target, silence_multiple, p.first, p.second);
	}
}
