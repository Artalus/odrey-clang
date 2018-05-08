#pragma once
#include <string>

#include "odr.h"

#include <clang/ASTMatchers/ASTMatchers.h>
#include <clang/ASTMatchers/ASTMatchFinder.h>


#include "clang/AST/AST.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/ASTConsumers.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Tooling/Tooling.h"
#include "clang/Rewrite/Core/Rewriter.h"


class OdrVisitor : public clang::RecursiveASTVisitor<OdrVisitor> {
	clang::ASTContext *astContext;
	clang::SourceManager &sourceManager;
	OdrMap &map;

public:
	explicit OdrVisitor(clang::CompilerInstance *CI, OdrMap &map)
		: astContext(&(CI->getASTContext()))
		, sourceManager(CI->getSourceManager())
		, map(map)
	{}

	bool VisitFunctionDecl(clang::FunctionDecl *func);

	bool VisitStmt(clang::Stmt *st);

	bool VisitRecordDecl(clang::RecordDecl *rec);
};


class OdrASTConsumer : public clang::ASTConsumer {
	OdrVisitor visitor;

public:
	explicit OdrASTConsumer(clang::CompilerInstance *CI, OdrMap &map)
		: visitor(CI, map)
	{ }

	virtual void HandleTranslationUnit(clang::ASTContext &Context) {
		visitor.TraverseDecl(Context.getTranslationUnitDecl());
	}
};


class OdrFrontendAction : public clang::ASTFrontendAction {
	OdrMap &map;
public:
	OdrFrontendAction(OdrMap &map)
		: map(map)
	{}

	std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(clang::CompilerInstance &CI, clang::StringRef file) override {
		return std::make_unique<OdrASTConsumer>(&CI, map); // pass CI pointer to ASTConsumer
	}
};

class OdrActionFactory : public clang::tooling::FrontendActionFactory {
	OdrMap &map;
public:
	OdrActionFactory(OdrMap &map)
		: map(map)
	{}

	/// The caller takes ownership of the returned action.
	virtual clang::FrontendAction *create(){
		return new OdrFrontendAction(map);
	}
};
