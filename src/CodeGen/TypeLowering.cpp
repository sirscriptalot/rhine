#include "rhine/IR/Context.hpp"
#include "rhine/IR/Type.hpp"
#include "rhine/IR/Value.hpp"

namespace rhine {
llvm::Type *UnType::toLL(llvm::Module *M) {
  assert(0 && "Cannot toLL() without inferring type");
  return nullptr;
}

llvm::Type *VoidType::toLL(llvm::Module *M) {
  return context()->Builder->getVoidTy();
}

llvm::Type *IntegerType::toLL(llvm::Module *M) {
  switch (Bitwidth) {
  case 32:
    return context()->Builder->getInt32Ty();
  case 64:
    return context()->Builder->getInt64Ty();
  default:
    assert (0 && "int bitwidths other than 32 and 64 are unimplemented");
  }
  return nullptr;
}

llvm::Type *BoolType::toLL(llvm::Module *M) {
  return context()->Builder->getInt1Ty();
}

llvm::Type *FloatType::toLL(llvm::Module *M) {
  return context()->Builder->getFloatTy();
}

llvm::Type *StringType::toLL(llvm::Module *M) {
  return context()->Builder->getInt8PtrTy();
}

llvm::Type *FunctionType::toLL(llvm::Module *M) {
  auto ATys = getATys();
  std::vector<llvm::Type *> ATyAr;
  if (ATys.size() != 1 || !isa<VoidType>(ATys[0]))
    for (auto Ty: getATys())
      ATyAr.push_back(Ty->toLL(M));
  return llvm::FunctionType::get(returnType()->toLL(M), ATyAr, isVariadic());
}

llvm::Type *PointerType::toLL(llvm::Module *M) {
  return llvm::PointerType::get(containedType()->toLL(M), 0);
}
}
