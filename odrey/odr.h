#pragma once
#include <set>

#include <clang/AST/Decl.h>
#include <clang/AST/ASTContext.h>
#include <clang/AST/Stmt.h>


struct Body {
	std::string text;
	clang::Stmt* body_statement;
	clang::ASTContext &ast_context;
	Body(const clang::Decl *d, clang::Stmt* stmt)
	    : text(to_str(d))
	    , body_statement(stmt)
	    , ast_context(d->getASTContext())
	{
	clang::Stmt::StmtClass sc1{};
	if (stmt)
		sc1 = stmt->getStmtClass();
	int x = 0;
	x *= 2;}
	virtual ~Body() = default;
	virtual std::string_view type() const = 0;

	static std::string to_str(const clang::Decl* d) {
		std::string result;
		llvm::raw_string_ostream writer(result);
		d->print(writer, 2);
		return result;
	}

	bool equal_to(const Body& rhs);
};

struct Function : Body {
	const clang::FunctionDecl* f;
	Function(const clang::FunctionDecl *f) : Body(f, f->getBody()), f(f) {}
	std::string_view type() const override { return "function"; }
};
struct Record : Body{
	const clang::RecordDecl* r;
	Record(const clang::RecordDecl *r) : Body(r, r->getBody()), r(r) {}
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

	bool equal_by_body_statement(const PrimaryEntry &rhs) const {
		return body->equal_to(*rhs.body);
	}
	static bool equal_by_body_statement(const PrimaryEntry& lhs, const PrimaryEntry &rhs) {
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
