#define DEBUG_TYPE "llvm_lx"
#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/Triple.h"
#include "llvm/Analysis/BasicAliasAnalysis.h"
#include "llvm/Analysis/CallGraph.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/Analysis/TypeBasedAliasAnalysis.h"
#include "llvm/AsmParser/Parser.h"
//#include "llvm/Bitcode/ReaderWriter.h"
#include "llvm/Bitcode/BitcodeWriter.h"
#include "llvm/CodeGen/LinkAllAsmWriterComponents.h"
#include "llvm/CodeGen/LinkAllCodegenComponents.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/IRPrintingPasses.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Verifier.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Linker/Linker.h"
#include "llvm/Linker/Linker.h"
#include "llvm/MC/SubtargetFeature.h"
#include "llvm/Pass.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/FileUtilities.h"
#include "llvm/Support/FormattedStream.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/PrettyStackTrace.h"
#include "llvm/Support/Program.h"
#include "llvm/Support/Signals.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/ToolOutputFile.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Transforms/Scalar.h"

#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/Passes.h"

#include <memory>
#include <string>

#include "TargetLoopExtractor.h"


using namespace std;
using namespace llvm;
using namespace llvm::sys;

cl::OptionCategory LLVMLXOptionCategory("Target Loop Extractor Options",
                                         "Additional options for the llvm-lx tool");

cl::list<string> LLVMLXLocations("t", cl::desc("List of loops to extract.\nFormat -t=<file1>:<line1>,<file2>:<line2>"),
                                    cl::Required, cl::cat(LLVMLXOptionCategory));

cl::opt<string> inPath(cl::Positional, cl::desc("<Module to analyze>"),
                       cl::value_desc("bitcode filename"), cl::Required,
                       cl::cat(LLVMLXOptionCategory));

static void saveModule(Module &m, StringRef filename) {
    error_code EC;
    raw_fd_ostream out(filename.data(), EC, sys::fs::F_None);

    if (EC) {
        report_fatal_error("error saving llvm module to '" + filename +
                           "': \n" + EC.message());
    }
    WriteBitcodeToFile(&m, out);
}

static void instrumentModule(Module &module) {
    // Build up all of the passes that we want to run on the module.
    legacy::PassManager pm;
    pm.add(new llvm::AssumptionCacheTracker());
    pm.add(llvm::createBasicAAWrapperPass());
    pm.add(createTypeBasedAAWrapperPass());
    pm.add(createBreakCriticalEdgesPass());
    pm.add(createLoopSimplifyPass());
    pm.add(new LoopInfoWrapperPass());
    pm.add(createVerifierPass());
    pm.add(new lx::TargetLoopExtractor());
    pm.run(module);

    auto replaceExt = [](string &s, const string &newExt) {
        string::size_type i = s.rfind('.', s.length());
        if (i != string::npos) {
            s.replace(i + 1, newExt.length(), newExt);
        }
    };

    replaceExt(inPath, "lx.bc");
    saveModule(module, inPath);
}


int main(int argc, char **argv) {
    // This boilerplate provides convenient stack traces and clean LLVM exit
    // handling. It also initializes the built in support for convenient
    // command line option handling.
    sys::PrintStackTraceOnErrorSignal(argv[0]);
    llvm::PrettyStackTraceProgram X(argc, argv);
    //LLVMContext &context = getGlobalContext();
    LLVMContext context;
    llvm_shutdown_obj shutdown;

    InitializeAllTargets();
    InitializeAllTargetMCs();
    InitializeAllAsmPrinters();
    InitializeAllAsmParsers();
    cl::AddExtraVersionPrinter(
        TargetRegistry::printRegisteredTargetsForVersion);
    cl::ParseCommandLineOptions(argc, argv);

    // Construct an IR file from the filename passed on the command line.
    SMDiagnostic err;
    unique_ptr<Module> module = parseIRFile(inPath.getValue(), err, context);

    if (!module.get()) {
        errs() << "Error reading bitcode file.\n";
        err.print(argv[0], errs());
        return -1;
    }

    instrumentModule(*module);

    return 0;
}
