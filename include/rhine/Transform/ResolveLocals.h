//-*- C++ -*-
#ifndef RHINE_RESOLVELOCALS_H
#define RHINE_RESOLVELOCALS_H

#include "rhine/Pass/FunctionPass.h"
#include "rhine/IR/Constant.h"

namespace rhine {
struct ValueRef {
  Value *Val;
  llvm::Value *LLVal;
  ValueRef(Value *Val, llvm::Value *LLVal) : Val(Val), LLVal(LLVal) {}
};

class ResolveLocals : public FunctionPass {
public:
  virtual ~ResolveLocals() {}
  void runOnFunction(Function *F) override;
private:
  std::vector<BasicBlock *> getBlocksInScope(BasicBlock *BB);
  void lookupReplaceUse(std::string Name, Use &U,
                        BasicBlock *Block);
  Value *lookupNameinBlock(std::string Name, BasicBlock *BB);
};
}

#endif
