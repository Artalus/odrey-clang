#include <llvm/Support/Error.h>
#include <llvm/Support/CommandLine.h>

#include "odr_tool.h"

namespace {

using namespace llvm::cl;

OptionCategory marker_category{"marker options"};

//TODO: why does it cuts "output e" if the string is not a separate variable? o_O
opt<std::string> compilation_database{"p", desc("path to directory containing project compilation database (compiler_commands.json)"),
	        init("."), cat(marker_category)};
list<std::string> input_filenames{Positional, desc("[<source0> [... <sourceN>]]"), ZeroOrMore};

}

int main(int argc, const char **argv) try {
	HideUnrelatedOptions(marker_category);
	ParseCommandLineOptions(argc, argv,
	                        "\n  This tool is used to insert a const char[] leak mark to each struct in project."
	                        "\n  Use positional arguments to specify files which should be instrumented."
	                        "\n  If no positional arguments are specified, the tool is applied to each file"
	                        "\n  in compilation database.\n");

	/*if (!show_output && !overwrite) {
		PrintHelpMessage();
		return 0;
	}*/

	OdrTool tool(compilation_database, input_filenames);

	return tool.check();
} catch (const llvm::Error &) {
	llvm::errs() << "Whoops! Error. How do I handle them in LLVM?\n";
	return 1;
} catch (const std::exception &e) {
	llvm::errs() << e.what() << '\n';
	return 1;
}
