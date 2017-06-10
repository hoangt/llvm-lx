#ifndef EPPPATHPRINTER_H
#define EPPPATHPRINTER_H

#include "llvm/ADT/APInt.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"

#include <set>


namespace lx {

struct TargetLoopExtractor : public llvm::ModulePass {
    static char ID;

    std::set<std::pair<std::string, int>> Locations;

    TargetLoopExtractor() : llvm::ModulePass(ID) {}

    virtual void getAnalysisUsage(llvm::AnalysisUsage &au) const override {
    }

    virtual bool runOnModule(llvm::Module &m) override;
    bool doInitialization(llvm::Module &m) override;
    void releaseMemory() override;

};


}

#endif
