#include "rhine/Externals.h"
#include "rhine/IR/Constant.h"
#include "rhine/IR/Context.h"
#include "rhine/IR/Instruction.h"

namespace rhine {
std::vector<std::pair<BasicBlock *, llvm::BasicBlock *>>
BasicBlock::zipSuccContainers(llvm::Module *M) {
  std::vector<std::pair<BasicBlock *, llvm::BasicBlock *>> SuccContainers;
  for (auto BB : succs()) {
    SuccContainers.push_back(std::make_pair(BB, BB->toContainerLL(M)));
  }
  return SuccContainers;
}

llvm::Value *BasicBlock::getPhiValueFromBranchBlock(llvm::Module *M) {
  auto K = getContext();
  assert(succ_size() == 2 && "Only 2 successors allowed for now");
  auto BranchBlock = K->Builder->GetInsertBlock();
  auto BranchContainers = zipSuccContainers(M);
  auto MergeBlockContainer = getMergeBlock()->toContainerLL(M);
  MergeBlockContainer->setName("merge");
  auto BrInst = cast<IfInst>(back());
  K->Builder->SetInsertPoint(BranchBlock);
  K->Builder->CreateCondBr(BrInst->getConditional()->toLL(M),
                           BranchContainers[0].second,
                           BranchContainers[1].second);
  std::vector<std::pair<llvm::BasicBlock *, llvm::Value *>> PhiIncoming;
  for (auto BBContainer : BranchContainers) {
    K->Builder->SetInsertPoint(BBContainer.second);
    auto BlockValue = BBContainer.first->toValuesLL(M);
    K->Builder->CreateBr(MergeBlockContainer);
    PhiIncoming.push_back(
        std::make_pair(K->Builder->GetInsertBlock(), BlockValue));
  }
  K->Builder->SetInsertPoint(MergeBlockContainer);
  auto VTy = back()->getType();
  if (!isa<VoidType>(VTy)) {
    auto PN = K->Builder->CreatePHI(VTy->toLL(M), 2, "iftmp");
    for (auto In : PhiIncoming) {
      PN->addIncoming(In.second, In.first);
    }
    return PN;
  }
  return nullptr;
}

llvm::Value *BasicBlock::toValuesLL(llvm::Module *M) {
  if (StmList.begin() == StmList.end())
    return nullptr;
  std::vector<Value *>::iterator It;
  for (It = StmList.begin(); std::next(It) != StmList.end(); ++It)
    (*It)->toLL(M);
  return (*It)->toLL(M);
}

llvm::BasicBlock *BasicBlock::toContainerLL(llvm::Module *M) {
  auto K = getContext();
  auto ParentF = cast<llvm::Function>(K->Map.getl(Parent));
  auto Ret = llvm::BasicBlock::Create(K->LLContext, Name, ParentF);
  K->Builder->SetInsertPoint(Ret);
  return Ret;
}

llvm::Value *BasicBlock::toLL(llvm::Module *M) {
  toContainerLL(M);
  return toValuesLL(M);
}
}
