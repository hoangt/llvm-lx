#define DEBUG_TYPE "lx"

#include "llvm/ADT/SmallString.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/CallSite.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DebugInfo.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include <fstream>

#include "TargetLoopExtractor.h"

using namespace llvm;
using namespace std;
using namespace lx;

extern cl::list<string> LLVMLXLocations;

bool TargetLoopExtractor::doInitialization(Module &M) { 
    for(auto Loc : LLVMLXLocations) {
        if(Loc.find(":") == string::npos || Loc.find(":") != Loc.rfind(":")) {
            report_fatal_error("Specify locations in the form of -t=<file1>:<line1>,<file2>:<line2> ...");
        }
        auto File = Loc.substr(0, Loc.find(":"));
        auto Line = stoi(Loc.substr(Loc.find(":")+1));
        Locations.insert({File, Line});
    }
    return false; 
}

bool TargetLoopExtractor::runOnModule(Module &M) {
    for(auto L : Locations) {
        errs() << L.first << " " << L.second << "\n";
    }
    
    for(auto &F : M) {

        if(F.isDeclaration()) continue;

        auto &LI = getAnalysis<LoopInfoWrapperPass>(F).getLoopInfo();          
        
        for(auto &L : LI) {
            auto Loc = L->getStartLoc();
            errs() << Loc.getLine()
        }
        
    }

    return false;
}

void TargetLoopExtractor::releaseMemory() {
    Locations.clear();
}

char TargetLoopExtractor::ID = 0;
