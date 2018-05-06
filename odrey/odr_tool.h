#pragma once

#include <string>
#include <vector>

#include <clang/Tooling/Tooling.h>

#include "odr_decl_traverser.h"


class OdrTool {
public:
	OdrTool(std::string db_path, std::vector<std::string> input_filenames);

	int check();

private:
	int apply_check();

	std::unique_ptr<clang::tooling::CompilationDatabase> db;
	std::vector<std::string> source_files;
	std::unique_ptr<clang::tooling::ClangTool> tool;

	OdrMap map;
};
