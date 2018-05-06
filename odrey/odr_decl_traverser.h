#pragma once
#include <string>

#include "odr.h"

#include <clang/ASTMatchers/ASTMatchers.h>
#include <clang/ASTMatchers/ASTMatchFinder.h>


class OdrDeclTraverser : public clang::ast_matchers::MatchFinder::MatchCallback {
protected:
	OdrMap &map;
public:
	OdrDeclTraverser(OdrMap &map)
	    : map(map)
	{}
	struct bind_names {
		static constexpr char rec[] = "rec";
		static constexpr char func[] = "func";
	};
};

class OdrRecordTraverser : public OdrDeclTraverser {
public:
	using OdrDeclTraverser::OdrDeclTraverser;
	void run(const clang::ast_matchers::MatchFinder::MatchResult &result) override;
};

class OdrFunctionTraverser : public OdrDeclTraverser {
public:
	using OdrDeclTraverser::OdrDeclTraverser;
	void run(const clang::ast_matchers::MatchFinder::MatchResult &result) override;
};
