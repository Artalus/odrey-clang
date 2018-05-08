#include "odr_tool.h"

#include <clang/Frontend/TextDiagnosticPrinter.h>


using namespace clang;
using namespace clang::tooling;

OdrTool::OdrTool(std::string db_path, std::vector<std::string> input_filenames) {
	std::string err = "errmsg??";
	db = CompilationDatabase::autoDetectFromDirectory(db_path, err);
	if (!db)
		throw std::runtime_error(err);

	this->source_files = input_filenames.empty()
	        ? db->getAllFiles()
	        : input_filenames;
	tool = std::make_unique<ClangTool>(*db, source_files);
}

int OdrTool::check()
{
	if (int result = tool->run(&odrFactory))
		throw std::runtime_error("tool.run() returned " + std::to_string(result) + "\n");

	return apply_check();
}

int OdrTool::apply_check()
{
	// We need a SourceManager to set up the Rewriter.
	IntrusiveRefCntPtr<DiagnosticOptions> diag_opts = new DiagnosticOptions();
	DiagnosticsEngine diagnostics(
	            IntrusiveRefCntPtr<DiagnosticIDs>(new DiagnosticIDs()),
	            &*diag_opts,
	            new TextDiagnosticPrinter(llvm::errs(), &*diag_opts), true);
	SourceManager sm(diagnostics, tool->getFiles());

//	for (const auto &x : map.entries_by_line)
//		std::cout << x;

	map.transform_entries();
	map.report_duplicates(llvm::errs());

	return 0;
}
