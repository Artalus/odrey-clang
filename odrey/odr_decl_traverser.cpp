#include "odr_decl_traverser.h"

#include <sstream>
#include <algorithm>

using std::string;
using std::stringstream;

using namespace clang;
using namespace clang::ast_matchers;

namespace {

template<class DeclType>
void parse(const DeclType* rec, OdrMap &map, const SourceManager &sm, void *addr) {
	SourceLocation rec_pos = rec->getLocStart();

	if (sm.isInExternCSystemHeader(rec_pos) || sm.isInSystemHeader(rec_pos))
		return;

	if (rec->isInAnonymousNamespace())
		return;


	map.emplace(rec_pos.printToString(sm),
	            rec->getQualifiedNameAsString(),
	            rec);
}

bool isMethodOfClass(const FunctionDecl *fun) {
	auto kind = fun->getKind();
	using K = FunctionDecl::Kind;
	return kind == K::CXXConstructor
	        || kind == K::CXXDestructor
	        || kind == K::CXXMethod;
}

bool strEndsWith(const std::string& str, const std::string &ends) {
	auto slen = str.length();
	auto elen = ends.length();
	if (slen < elen)
		return false;
	return (str.compare(slen - elen, elen, ends) == 0);
}

}

bool OdrVisitor::VisitFunctionDecl(FunctionDecl *fun) {
	if (fun->doesThisDeclarationHaveABody()
			&& fun->isInlined()
			&& !fun->isConstexpr()
			&& !strEndsWith(fun->getQualifiedNameAsString(), "(anonymous struct)::")
		)
		parse(fun, map, sourceManager, fun->getBody());
	return true;
}

bool OdrVisitor::VisitStmt(clang::Stmt *st) {
	return true;
}

bool OdrVisitor::VisitRecordDecl(RecordDecl *rec) {
	if (rec->isCompleteDefinition()
		&& !strEndsWith(rec->getQualifiedNameAsString(), "(anonymous)")
		)
		parse(rec, map, sourceManager, rec->getDefinition());
	return true;
}
