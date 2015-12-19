#include "rhine/IR/Instruction.hpp"

namespace rhine {
Instruction::Instruction(Type *Ty, RTValue ID, unsigned NumOps, std::string N)
    : User(Ty, ID, NumOps, N) {}

bool Instruction::classof(const Value *V) {
  return V->getValID() >= RT_AddInst && V->getValID() <= RT_IfInst;
}

BasicBlock *Instruction::getParent() const { return Parent; }

void Instruction::setParent(BasicBlock *P) { Parent = P; }

BinaryArithInst::BinaryArithInst(RTValue InstSelector, Type *Ty, Value *Op0,
                                 Value *Op1)
    : Instruction(Ty, InstSelector, 2) {
  setOperand(0, Op0);
  setOperand(1, Op1);
}

void *BinaryArithInst::operator new(size_t S) {
  return User::operator new(S, 2);
}

BinaryArithInst *BinaryArithInst::get(RTValue InstSelector, Value *Op0,
                                      Value *Op1) {
  auto K = Op0->getContext();
  return new BinaryArithInst(InstSelector, UnType::get(K), Op0, Op1);
}

bool BinaryArithInst::classof(const Value *V) {
  return V->getValID() == RT_MulInst || V->getValID() == RT_DivInst ||
  V->getValID() == RT_AddInst || V->getValID() == RT_SubInst;
}

void BinaryArithInst::print(std::ostream &Stream) const {
  switch (getValID()) {
  case RT_AddInst:
    Stream << "+ ~" << *VTy;
    break;
  case RT_SubInst:
    Stream << "- ~" << *VTy;
    break;
  case RT_MulInst:
    Stream << "* ~" << *VTy;
    break;
  case RT_DivInst:
    Stream << "/ ~" << *VTy;
    break;
  default:
    assert(0 && "Malformed BinaryArithInst; cannot print");
  }
  for (auto O : getOperands())
    Stream << std::endl << *O;
}

CallInst::CallInst(Type *Ty, unsigned NumOps, std::string N)
    : Instruction(Ty, RT_CallInst, NumOps, N) {}

CallInst::~CallInst() {}

void *CallInst::operator new(size_t S, unsigned N) {
  return User::operator new(S, N);
}

CallInst *CallInst::get(Value *Callee, std::vector<Value *> Ops) {
  auto NumOps = Ops.size();
  auto Name = Callee->getName();
  auto Obj = new (NumOps + 1) CallInst(Callee->getType(), NumOps, Name);
  Obj->NumAllocatedOps = NumOps + 1;
  Obj->setOperand(-1, Callee);
  for (unsigned OpN = 0; OpN < NumOps; OpN++)
    Obj->setOperand(OpN, Ops[OpN]);
  return Obj;
}

bool CallInst::classof(const Value *V) { return V->getValID() == RT_CallInst; }

FunctionType *CallInst::getFTy() const {
  auto PTy = cast<PointerType>(getCallee()->getType());
  assert(PTy && "Illegal call to getFTy() before type inference");
  return cast<FunctionType>(PTy->getCTy());
}

std::vector<Type *> CallInst::getATys() const {
  return getFTy()->getATys();
}

Type *CallInst::getRTy() const {
  return getFTy()->getRTy();
}

Value *CallInst::getCallee() const { return getOperand(-1); }

void CallInst::print(std::ostream &Stream) const {
  Stream << getCallee()->getName() << " ~" << *VTy;
  for (auto O : getOperands())
    Stream << std::endl << *O;
}

MallocInst::MallocInst(std::string N, Value *V)
    : Instruction(V->getType(), RT_MallocInst, 1, N) {
  setOperand(0, V);
}

MallocInst::~MallocInst() {}

void *MallocInst::operator new(size_t S) { return User::operator new(S, 1); }

MallocInst *MallocInst::get(std::string N, Value *V) {
  return new MallocInst(N, V);
}

bool MallocInst::classof(const Value *V) {
  return V->getValID() == RT_MallocInst;
}

void MallocInst::setVal(Value *V) { setOperand(0, V); }

Value *MallocInst::getVal() { return getOperand(0); }

void MallocInst::print(std::ostream &Stream) const {
  Stream << Name << " = " << *getOperand(0);
}

LoadInst::LoadInst(MallocInst *M)
    : Instruction(M->getType(), RT_LoadInst, 1, M->getName()) {
  setOperand(0, M);
}

void *LoadInst::operator new(size_t S) { return User::operator new(S, 1); }

LoadInst *LoadInst::get(MallocInst *M) { return new LoadInst(M); }

bool LoadInst::classof(const Value *V) { return V->getValID() == RT_LoadInst; }

Value *LoadInst::getVal() const { return getOperand(0); }

void LoadInst::print(std::ostream &Stream) const {
  Stream << Name << " ~" << *VTy;
}

StoreInst::StoreInst(Value *MallocedValue, Value *NewValue)
    : Instruction(MallocedValue->getType(), RT_StoreInst, 2) {
  assert(MallocedValue->getType() == NewValue->getType() &&
         "Type mismatch in store");
  setOperand(0, MallocedValue);
  setOperand(1, NewValue);
}

StoreInst::~StoreInst() {}

void *StoreInst::operator new(size_t S) { return User::operator new(S, 2); }

StoreInst *StoreInst::get(Value *MallocedValue, Value *NewValue) {
  return new StoreInst(MallocedValue, NewValue);
}

bool StoreInst::classof(const Value *V) {
  return V->getValID() == RT_StoreInst;
}

Value *StoreInst::getMallocedValue() const { return getOperand(0); }

Value *StoreInst::getNewValue() const { return getOperand(1); }

void StoreInst::print(std::ostream &Stream) const {
  Stream << *getMallocedValue() << " = " << *getNewValue();
}

ReturnInst::ReturnInst(Type *Ty, bool IsVoid)
    : Instruction(Ty, RT_ReturnInst, !IsVoid) {}

ReturnInst::~ReturnInst() {}

void *ReturnInst::operator new(size_t S, unsigned N) {
  return User::operator new(S, N);
}

ReturnInst *ReturnInst::get(Value *V, Context *K) {
  if (!V)
    return new (0) ReturnInst(VoidType::get(K), true);
  auto Obj = new (1) ReturnInst(V->getType(), false);
  Obj->setOperand(0, V);
  return Obj;
}

bool ReturnInst::classof(const Value *V) {
  return V->getValID() == RT_ReturnInst;
}

void ReturnInst::setVal(Value *V) { setOperand(0, V); }

Value *ReturnInst::getVal() {
  auto Operands = getOperands();
  if (Operands.size())
    return Operands[0];
  return nullptr;
}

void ReturnInst::print(std::ostream &Stream) const {
  Stream << "ret " << *getOperand(0);
}

TerminatorInst::TerminatorInst(Type *Ty)
    : Instruction(Ty, RT_TerminatorInst, 1) {}

TerminatorInst::~TerminatorInst() {}

void *TerminatorInst::operator new(size_t S) {
  return User::operator new(S, 1);
}

TerminatorInst *TerminatorInst::get(Value *V) {
  auto Obj = new TerminatorInst(V->getType());
  Obj->setOperand(0, V);
  return Obj;
}

bool TerminatorInst::classof(const Value *V) {
  return V->getValID() == RT_TerminatorInst;
}

void TerminatorInst::setVal(Value *V) { setOperand(0, V); }

Value *TerminatorInst::getVal() { return getOperand(0); }

void TerminatorInst::print(std::ostream &Stream) const {
  Stream << "term " << *getOperand(0);
}

IfInst::IfInst(Type *Ty) : Instruction(Ty, RT_IfInst, 3) {}

void *IfInst::operator new(size_t S) { return User::operator new(S, 3); }

IfInst *IfInst::get(Value *Conditional, BasicBlock *TrueBB,
                    BasicBlock *FalseBB) {
  auto Obj = new IfInst(UnType::get(Conditional->getContext()));
  TrueBB->setName("true");
  FalseBB->setName("false");
  Obj->setOperand(0, Conditional);
  Obj->setOperand(1, TrueBB);
  Obj->setOperand(2, FalseBB);
  return Obj;
}

bool IfInst::classof(const Value *V) { return V->getValID() == RT_IfInst; }

Value *IfInst::getConditional() const { return getOperand(0); }

BasicBlock *IfInst::getTrueBB() const {
  return cast<BasicBlock>(getOperand(1));
}
BasicBlock *IfInst::getFalseBB() const {
  return cast<BasicBlock>(getOperand(2));
}

void IfInst::print(std::ostream &Stream) const {
  Stream << "if (" << *getConditional() << ") {" << std::endl;
  for (auto V : *getTrueBB())
    Stream << *V << std::endl;
  Stream << "} else {" << std::endl;
  for (auto V : *getFalseBB())
    Stream << *V << std::endl;
  Stream << "}";
}
}
