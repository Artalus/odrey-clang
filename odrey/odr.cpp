#include "odr.h"

#include <iterator>
#include <sstream>

#include <clang/AST/Expr.h>

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
	    [&first](const PrimaryEntry &e) { return !e.equal_by_body_statement(first); });
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

// from clang::IdenticalExprChecker.cpp
namespace clang {
namespace {
static bool isIdenticalStmt(const ASTContext &Ctx, const Stmt *Stmt1,
                            const Stmt *Stmt2, bool IgnoreSideEffects) {

	if (!Stmt1 || !Stmt2) {
		return !Stmt1 && !Stmt2;
	}

	// If Stmt1 & Stmt2 are of different class then they are not
	// identical statements.
	auto sc1 = Stmt1->getStmtClass();
	auto sc2 = Stmt2->getStmtClass();
	if (sc1 != sc2)
		return false;

	const Expr *Expr1 = dyn_cast<Expr>(Stmt1);
	const Expr *Expr2 = dyn_cast<Expr>(Stmt2);

	if (Expr1 && Expr2) {
		// If Stmt1 has side effects then don't warn even if expressions
		// are identical.
		if (!IgnoreSideEffects && Expr1->HasSideEffects(Ctx))
			return false;
		// If either expression comes from a macro then don't warn even if
		// the expressions are identical.
		if ((Expr1->getExprLoc().isMacroID()) || (Expr2->getExprLoc().isMacroID()))
			return false;

		// If all children of two expressions are identical, return true.
		Expr::const_child_iterator I1 = Expr1->child_begin();
		Expr::const_child_iterator I2 = Expr2->child_begin();
		while (I1 != Expr1->child_end() && I2 != Expr2->child_end()) {
			if (!*I1 || !*I2 || !isIdenticalStmt(Ctx, *I1, *I2, IgnoreSideEffects))
				return false;
			++I1;
			++I2;
		}
		// If there are different number of children in the statements, return
		// false.
		if (I1 != Expr1->child_end())
			return false;
		if (I2 != Expr2->child_end())
			return false;
	}

	switch (Stmt1->getStmtClass()) {
	default:
		return false;
	case Stmt::CallExprClass:
	case Stmt::ArraySubscriptExprClass:
	case Stmt::OMPArraySectionExprClass:
	case Stmt::ImplicitCastExprClass:
	case Stmt::ParenExprClass:
	case Stmt::BreakStmtClass:
	case Stmt::ContinueStmtClass:
	case Stmt::NullStmtClass:
		return true;
	case Stmt::CStyleCastExprClass: {
		const CStyleCastExpr* CastExpr1 = cast<CStyleCastExpr>(Stmt1);
		const CStyleCastExpr* CastExpr2 = cast<CStyleCastExpr>(Stmt2);

		return CastExpr1->getTypeAsWritten() == CastExpr2->getTypeAsWritten();
	}
	case Stmt::ReturnStmtClass: {
		const ReturnStmt *ReturnStmt1 = cast<ReturnStmt>(Stmt1);
		const ReturnStmt *ReturnStmt2 = cast<ReturnStmt>(Stmt2);

		return isIdenticalStmt(Ctx, ReturnStmt1->getRetValue(),
		                       ReturnStmt2->getRetValue(), IgnoreSideEffects);
	}
	case Stmt::ForStmtClass: {
		const ForStmt *ForStmt1 = cast<ForStmt>(Stmt1);
		const ForStmt *ForStmt2 = cast<ForStmt>(Stmt2);

		if (!isIdenticalStmt(Ctx, ForStmt1->getInit(), ForStmt2->getInit(),
		                     IgnoreSideEffects))
			return false;
		if (!isIdenticalStmt(Ctx, ForStmt1->getCond(), ForStmt2->getCond(),
		                     IgnoreSideEffects))
			return false;
		if (!isIdenticalStmt(Ctx, ForStmt1->getInc(), ForStmt2->getInc(),
		                     IgnoreSideEffects))
			return false;
		if (!isIdenticalStmt(Ctx, ForStmt1->getBody(), ForStmt2->getBody(),
		                     IgnoreSideEffects))
			return false;
		return true;
	}
	case Stmt::DoStmtClass: {
		const DoStmt *DStmt1 = cast<DoStmt>(Stmt1);
		const DoStmt *DStmt2 = cast<DoStmt>(Stmt2);

		if (!isIdenticalStmt(Ctx, DStmt1->getCond(), DStmt2->getCond(),
		                     IgnoreSideEffects))
			return false;
		if (!isIdenticalStmt(Ctx, DStmt1->getBody(), DStmt2->getBody(),
		                     IgnoreSideEffects))
			return false;
		return true;
	}
	case Stmt::WhileStmtClass: {
		const WhileStmt *WStmt1 = cast<WhileStmt>(Stmt1);
		const WhileStmt *WStmt2 = cast<WhileStmt>(Stmt2);

		if (!isIdenticalStmt(Ctx, WStmt1->getCond(), WStmt2->getCond(),
		                     IgnoreSideEffects))
			return false;
		if (!isIdenticalStmt(Ctx, WStmt1->getBody(), WStmt2->getBody(),
		                     IgnoreSideEffects))
			return false;
		return true;
	}
	case Stmt::IfStmtClass: {
		const IfStmt *IStmt1 = cast<IfStmt>(Stmt1);
		const IfStmt *IStmt2 = cast<IfStmt>(Stmt2);

		if (!isIdenticalStmt(Ctx, IStmt1->getCond(), IStmt2->getCond(),
		                     IgnoreSideEffects))
			return false;
		if (!isIdenticalStmt(Ctx, IStmt1->getThen(), IStmt2->getThen(),
		                     IgnoreSideEffects))
			return false;
		if (!isIdenticalStmt(Ctx, IStmt1->getElse(), IStmt2->getElse(),
		                     IgnoreSideEffects))
			return false;
		return true;
	}
	case Stmt::CompoundStmtClass: {
		const CompoundStmt *CompStmt1 = cast<CompoundStmt>(Stmt1);
		const CompoundStmt *CompStmt2 = cast<CompoundStmt>(Stmt2);

		if (CompStmt1->size() != CompStmt2->size())
			return false;

		CompoundStmt::const_body_iterator I1 = CompStmt1->body_begin();
		CompoundStmt::const_body_iterator I2 = CompStmt2->body_begin();
		while (I1 != CompStmt1->body_end() && I2 != CompStmt2->body_end()) {
			if (!isIdenticalStmt(Ctx, *I1, *I2, IgnoreSideEffects))
				return false;
			++I1;
			++I2;
		}

		return true;
	}
	case Stmt::CompoundAssignOperatorClass:
	case Stmt::BinaryOperatorClass: {
		const BinaryOperator *BinOp1 = cast<BinaryOperator>(Stmt1);
		const BinaryOperator *BinOp2 = cast<BinaryOperator>(Stmt2);
		return BinOp1->getOpcode() == BinOp2->getOpcode();
	}
	case Stmt::CharacterLiteralClass: {
		const CharacterLiteral *CharLit1 = cast<CharacterLiteral>(Stmt1);
		const CharacterLiteral *CharLit2 = cast<CharacterLiteral>(Stmt2);
		return CharLit1->getValue() == CharLit2->getValue();
	}
	case Stmt::DeclRefExprClass: {
		const DeclRefExpr *DeclRef1 = cast<DeclRefExpr>(Stmt1);
		const DeclRefExpr *DeclRef2 = cast<DeclRefExpr>(Stmt2);
		return DeclRef1->getDecl() == DeclRef2->getDecl();
	}
	case Stmt::IntegerLiteralClass: {
		const IntegerLiteral *IntLit1 = cast<IntegerLiteral>(Stmt1);
		const IntegerLiteral *IntLit2 = cast<IntegerLiteral>(Stmt2);

		llvm::APInt I1 = IntLit1->getValue();
		llvm::APInt I2 = IntLit2->getValue();
		if (I1.getBitWidth() != I2.getBitWidth())
			return false;
		return  I1 == I2;
	}
	case Stmt::FloatingLiteralClass: {
		const FloatingLiteral *FloatLit1 = cast<FloatingLiteral>(Stmt1);
		const FloatingLiteral *FloatLit2 = cast<FloatingLiteral>(Stmt2);
		return FloatLit1->getValue().bitwiseIsEqual(FloatLit2->getValue());
	}
	case Stmt::StringLiteralClass: {
		const StringLiteral *StringLit1 = cast<StringLiteral>(Stmt1);
		const StringLiteral *StringLit2 = cast<StringLiteral>(Stmt2);
		return StringLit1->getBytes() == StringLit2->getBytes();
	}
	case Stmt::MemberExprClass: {
		const MemberExpr *MemberStmt1 = cast<MemberExpr>(Stmt1);
		const MemberExpr *MemberStmt2 = cast<MemberExpr>(Stmt2);
		return MemberStmt1->getMemberDecl() == MemberStmt2->getMemberDecl();
	}
	case Stmt::UnaryOperatorClass: {
		const UnaryOperator *UnaryOp1 = cast<UnaryOperator>(Stmt1);
		const UnaryOperator *UnaryOp2 = cast<UnaryOperator>(Stmt2);
		return UnaryOp1->getOpcode() == UnaryOp2->getOpcode();
	}
	}
}
}
}
bool Body::equal_to(const Body &rhs){
	return clang::isIdenticalStmt(ast_context, body_statement, rhs.body_statement, false);
}
