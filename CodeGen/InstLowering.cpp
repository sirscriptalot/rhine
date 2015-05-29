#include "rhine/Context.h"
#include "rhine/Resolve.h"
#include "rhine/IR/Instruction.h"
#include "rhine/Externals.h"

namespace rhine {
llvm::Function *getCalleeFunction(std::string Name, location SourceLoc,
                                       llvm::Module *M, Context *K) {
  if (auto Result = K->getMappingVal(Name)) {
    if (auto CalleeCandidate = dyn_cast<llvm::Function>(Result))
      return CalleeCandidate;
    else {
      K->DiagPrinter->errorReport(
          SourceLoc, Name + " was not declared as a function");
      exit(1);
    }
  } else if (auto FPtr = Externals::get(K)->getMappingVal(Name)) {
    if (auto CalleeCandidate = dyn_cast<llvm::Function>(FPtr(M, K)))
      return CalleeCandidate;
    else {
      // Polymorphic externals?
      K->DiagPrinter->errorReport(
          SourceLoc, Name + " was declared with different signature earlier");
      exit(1);
    }
  }
  K->DiagPrinter->errorReport(
      SourceLoc, "unable to look up function " + Name);
  exit(1);
}

llvm::Value *CallInst::toLL(llvm::Module *M, Context *K) {
  auto Callee = getCalleeFunction(Name, SourceLoc, M, K);

  if (!getOperands().size())
    return K->Builder->CreateCall(Callee, Name);

  // Prepare arguments to call
  std::vector<llvm::Value *> LLOps;
  for (auto Op: getOperands()) {
    LLOps.push_back(Op->toLL(M, K));
  }
  return K->Builder->CreateCall(Callee, LLOps, Name);
}

llvm::Value *AddInst::toLL(llvm::Module *M, Context *K) {
  auto Op0 = getOperand(0)->toLL(M, K);
  auto Op1 = getOperand(1)->toLL(M, K);
  return K->Builder->CreateAdd(Op0, Op1);
}

llvm::Value *BindInst::toLL(llvm::Module *M, Context *K) {
  auto V = getVal()->toLL(M, K);
  K->addMapping(getName(), nullptr, V);
  return nullptr;
}
}
