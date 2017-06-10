#define DEBUG_TYPE "lx"

#include "llvm/ADT/SmallString.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Transforms/Utils/CodeExtractor.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/CallSite.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DebugInfo.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

#include "TargetLoopExtractor.h"

using namespace llvm;
using namespace std;
using namespace lx;

extern cl::list<string> LLVMLXLocations;
extern cl::opt<bool> LLVMLXDumpAllLoopLocations;

bool TargetLoopExtractor::doInitialization(Module &M) { 
    for(auto Loc : LLVMLXLocations) {
        if(Loc.find(":") == string::npos || Loc.find(":") != Loc.rfind(":")) {
            report_fatal_error("[llvm-lx] Specify locations in the form of -t=<file1>:<line1>,<file2>:<line2> ...");
        }
        auto File = Loc.substr(0, Loc.find(":"));
        auto Line = stoi(Loc.substr(Loc.find(":")+1));
        Locations.insert({File, Line});
    }

    if(LLVMLXDumpAllLoopLocations) 
        LoopLocationDumpFile.open("loop-locs.txt", ios::out);
   

    return false; 
}

bool TargetLoopExtractor::doFinalization(Module &M) { 

    for(auto Loc : Locations) {
        errs() << "[llvm-lx] Loop Not Found - " << Loc.first << " " << Loc.second << "\n";
    }

    if(LLVMLXDumpAllLoopLocations) 
        LoopLocationDumpFile.close();

    return false;
}

//! Adapted from lib/Transform/IPO/LoopExtractor.cpp
bool TargetLoopExtractor::extractLoop(Loop *L, 
        LoopInfo& LI, DominatorTree& DT, string Name) {

    bool ShouldExtractLoop = true;

    if(!L->isLoopSimplifyForm()) {
        errs() << "[llvm-lx] Loop not in Simplify Form\n";
        ShouldExtractLoop = false;
    } 

    // Split all the exit basic blocks to ensure that in case there is a 
    // exit basic block which has work done in the block before return, then 
    // we should split the block and leave the extra instructions outside the 
    // extracted function.
    

    SmallVector<BasicBlock*, 8> ExitBlocks;
    L->getExitBlocks(ExitBlocks);

    for(unsigned i = 0, e = ExitBlocks.size(); i != e; ++i) {
        SplitBlock(ExitBlocks[i], &*ExitBlocks[i]->getFirstInsertionPt(), &DT, &LI);
    }

    if(ShouldExtractLoop) {
        // We must omit landing pads. Landing pads must accompany the invoke
        // instruction. But this would result in a loop in the extracted
        // function. An infinite cycle occurs when it tries to extract that loop as
        // well.
        SmallVector<BasicBlock*, 8> ExitBlocks;
        L->getExitBlocks(ExitBlocks);
        for (unsigned i = 0, e = ExitBlocks.size(); i != e; ++i) {
            if (ExitBlocks[i]->isLandingPad()) {
                ShouldExtractLoop = false;
                errs() << "[llvm-lx] Omit Landing Pad Instructions\n";
                break;
            }
        }
    }

    if(ShouldExtractLoop) {
        CodeExtractor Extractor(DT, *L);
        Function *F = Extractor.extractCodeRegion(); 
        if (F) {
            F->setName(std::string("__lx") + Name);
            ExtractedLoopFunctions.insert(F);
            return true;
        }
    }

    return false;
}

bool TargetLoopExtractor::runOnModule(Module &M) {
    for(auto L : Locations) {
        errs() << L.first << " " << L.second << "\n";
    }
    
    for(auto &F : M) {

        if(F.isDeclaration() ||
                ExtractedLoopFunctions.count(&F)) continue;

        auto &LI = getAnalysis<LoopInfoWrapperPass>(F).getLoopInfo();          
        auto &DT = getAnalysis<DominatorTreeWrapperPass>(F).getDomTree();
        
        for(auto &L : LI) {
            auto Loc = L->getStartLoc();
            auto Filename = Loc->getFilename().str();
            auto Line = Loc.getLine();

            if(LLVMLXDumpAllLoopLocations) {
                LoopLocationDumpFile << Loc->getFilename().str() << ":" << Loc.getLine() << "\n";
            }

            if(Locations.count({Filename, Line})) {
                errs() << "[llvm-lx] Found loop at " << Filename << ":" << Line << "\n";
                if(!extractLoop(L, LI, DT, Filename+to_string(Line))) {
                    errs() << "[llvm-lx] Unable to extract loop at " << Filename << ":" << Line << "\n";
                }
                Locations.erase({Filename, Line});
            }
        }
    }

    return false;
}


char TargetLoopExtractor::ID = 0;
