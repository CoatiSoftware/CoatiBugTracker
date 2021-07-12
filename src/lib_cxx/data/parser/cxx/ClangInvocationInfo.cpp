#include "ClangInvocationInfo.h"

#include <clang/Driver/Compilation.h>
#include <clang/Driver/Driver.h>
#include <clang/Driver/Options.h>
#include <clang/Frontend/CompilerInvocation.h>
#include <clang/Tooling/Tooling.h>
#include <clang/Basic/Version.h>
#include <llvm/Option/ArgList.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/Host.h>

#include "CxxCompilationDatabaseSingle.h"
#include "CxxDiagnosticConsumer.h"
#include "utilityString.h"

namespace
{
// copied from clang codebase
clang::driver::Driver* newDriver(
	clang::DiagnosticsEngine* Diagnostics,
	const char* BinaryName,
	clang::IntrusiveRefCntPtr<llvm::vfs::FileSystem> VFS)
{
	#if CLANG_VERSION_MAJOR > 11
	clang::driver::Driver* CompilerDriver = new clang::driver::Driver(
		BinaryName, llvm::sys::getDefaultTargetTriple(), *Diagnostics,
		"clang_based_tool", std::move(VFS));
	#else
	clang::driver::Driver* CompilerDriver = new clang::driver::Driver(
		BinaryName, llvm::sys::getDefaultTargetTriple(), *Diagnostics,
		std::move(VFS));
	CompilerDriver->setTitle("clang_based_tool");
	#endif
	return CompilerDriver;
}
}	 // namespace

// copied and stitched together from clang codebase
ClangInvocationInfo ClangInvocationInfo::getClangInvocationString(
	const clang::tooling::CompilationDatabase* compilationDatabase)
{
	ClangInvocationInfo invocationInfo;

	if (!compilationDatabase->getAllCompileCommands().empty())
	{
		std::vector<std::string> CommandLine =
			compilationDatabase->getAllCompileCommands().front().CommandLine;

		std::vector<const char*> Argv;
		for (const std::string& Str: CommandLine)
			Argv.push_back(Str.c_str());
		const char* const BinaryName = Argv[0];
		clang::IntrusiveRefCntPtr<clang::DiagnosticOptions> DiagOpts = new clang::DiagnosticOptions();
		unsigned MissingArgIndex, MissingArgCount;
		llvm::opt::OptTable Opts = clang::driver::getDriverOptTable();
		llvm::opt::InputArgList ParsedArgs = Opts.ParseArgs(
			clang::ArrayRef<const char*>(Argv).slice(1), MissingArgIndex, MissingArgCount);
		clang::ParseDiagnosticArgs(*DiagOpts, ParsedArgs);

		llvm::raw_string_ostream diagnosticsStream(invocationInfo.errors);
		clang::TextDiagnosticPrinter DiagnosticPrinter(diagnosticsStream, &*DiagOpts);
		clang::DiagnosticsEngine Diagnostics(
			clang::IntrusiveRefCntPtr<clang::DiagnosticIDs>(new clang::DiagnosticIDs()),
			&*DiagOpts,
			&DiagnosticPrinter,
			false);

		llvm::IntrusiveRefCntPtr<clang::FileManager> Files(
			new clang::FileManager(clang::FileSystemOptions()));

		const std::unique_ptr<clang::driver::Driver> Driver(
			newDriver(&Diagnostics, BinaryName, &Files->getVirtualFileSystem()));
		// Since the input might only be virtual, don't check whether it exists.
		Driver->setCheckInputsExist(false);
		const std::unique_ptr<clang::driver::Compilation> Compilation(
			Driver->BuildCompilation(llvm::makeArrayRef(Argv)));

		if (Compilation)
		{
			llvm::raw_string_ostream ss(invocationInfo.invocation);
			Compilation->getJobs().Print(ss, "", true);
			ss.flush();
		}

		diagnosticsStream.flush();

		invocationInfo.invocation = utility::trim(invocationInfo.invocation);
		invocationInfo.errors = utility::trim(invocationInfo.errors);
	}
	return invocationInfo;
}
