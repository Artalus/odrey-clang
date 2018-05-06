#include "odr_decl_traverser.h"

#include <sstream>
#include <algorithm>

using std::string;
using std::stringstream;

using namespace clang;
using namespace clang::ast_matchers;

namespace {

template<class DeclType>
void parse(const DeclType* rec, OdrMap &map, const SourceManager *sm, void *addr) {
	SourceLocation rec_pos = rec->getLocStart();

	if (sm->isInExternCSystemHeader(rec_pos) || sm->isInSystemHeader(rec_pos))
		return;

	if (rec->isInAnonymousNamespace())
		return;


	map.emplace(rec_pos.printToString(*sm),
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

}

void OdrRecordTraverser::run(const MatchFinder::MatchResult &result) {
	auto *sm = result.SourceManager;
	if (const auto* rec = result.Nodes.getNodeAs<RecordDecl>(bind_names::rec)) {
		if (rec->isCompleteDefinition())
			return parse(rec, map, sm, rec->getDefinition());
	}
}

void OdrFunctionTraverser::run(const MatchFinder::MatchResult &result) {
	auto *sm = result.SourceManager;
	if (const auto* fun = result.Nodes.getNodeAs<FunctionDecl>(bind_names::func)) {
		if (fun->doesThisDeclarationHaveABody()
		        && fun->isInlined()
		        && !fun->isConstexpr())
			return parse(fun, map, sm, fun->getBody());
	}

}
