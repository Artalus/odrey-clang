#pragma once
#include <set>

#include <clang/AST/Decl.h>


struct Body {
	const std::string text;
	Body(const clang::Decl *d)
	    :text(to_str(d))
	{}
	virtual ~Body() = default;
	virtual std::string_view type() const = 0;

	static std::string to_str(const clang::Decl* d) {
		std::string result;
		llvm::raw_string_ostream writer(result);
		d->print(writer, 2);
		return result;
	}
};

struct Function : Body {
	Function(const clang::FunctionDecl *f) : Body(f) {}
	std::string_view type() const override { return "function"; }
};
struct Record : Body{
	Record(const clang::RecordDecl *r) : Body(r) {}
	std::string_view type() const override { return "record"; }
};

template<class SomeDecl> std::unique_ptr<Body> make_body(const SomeDecl*);

struct PrimaryEntry {
	std::string location;
	std::string name;
	std::unique_ptr<Body> body;
	template<class SomeDecl>
	PrimaryEntry(std::string location,
	         std::string name,
	         SomeDecl *d)
	    : location(move(location))
	    , name(move(name))
	    , body(make_body(d))
	{}
	PrimaryEntry(const PrimaryEntry&) = delete;
	PrimaryEntry(PrimaryEntry &&) = default;
	PrimaryEntry& operator=(PrimaryEntry &&) = default;

	struct less_by_location {
		bool operator()(const PrimaryEntry& lhs, const PrimaryEntry &rhs) const {
			return lhs.location < rhs.location;
		}
	};
	bool equal_by_body_text(const PrimaryEntry &rhs) const {
		return body->text == rhs.body->text;
	}
	static bool equal_by_body_text(const PrimaryEntry& lhs, const PrimaryEntry &rhs) {
		return lhs.equal_by_body_text(rhs);
	}
};

inline std::ostream& operator<<(std::ostream &s, const PrimaryEntry &x) {
	s << x.location << ": "<< x.name
	         << " ("<< x.body->type() <<")"
	         "\n"
	         << x.body->text << "\n";
	return s;
}

struct OdrMap {
	template<class...Args>
	void emplace(Args&&...a) {
		entries_by_line.emplace(std::forward<Args>(a)...);
	}

	void transform_entries();

	void report_duplicates(llvm::raw_ostream &target, bool silence_multiple=false);

	std::set<PrimaryEntry, PrimaryEntry::less_by_location> entries_by_line;
	std::map<std::string, std::list<PrimaryEntry>> name_to_definition;
};
