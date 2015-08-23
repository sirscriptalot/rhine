#include "rhine/IR/Instruction.h"

namespace rhine {
Instruction::Instruction(Type *Ty, RTValue ID,
                         unsigned NumOps, std::string N) :
    User(Ty, ID, NumOps, N) {}

bool Instruction::classof(const Value *V) {
  return V->getValID() >= RT_AddInst &&
    V->getValID() <= RT_IfInst;
}

std::vector<Value *> Instruction::getOperands() const {
  std::vector<Value *> OpV;
  for (unsigned OpN = 0; OpN < NumOperands; OpN++) {
    OpV.push_back(getOperand(OpN));
  }
  return OpV;
}

void Instruction::setOperands(std::vector<Value *> Ops) {
  assert(Ops.size() == NumOperands && "Incorrect number passed to setOperands()");
  for (unsigned OpN = 0; OpN < NumOperands; OpN++) {
    setOperand(OpN, Ops[OpN]);
  }
}

AddInst::AddInst(Type *Ty) : Instruction(Ty, RT_AddInst, 2) {}

void *AddInst::operator new(size_t s) {
  return User::operator new(s, 2);
}

AddInst *AddInst::get(Context *K) {
  return new AddInst(UnType::get(K));
}

bool AddInst::classof(const Value *V) {
  return V->getValID() == RT_AddInst;
}

void AddInst::print(std::ostream &Stream) const {
  Stream << "+ ~" << *VTy;
  for (auto O: getOperands())
    Stream << std::endl << *O;
}

CallInst::CallInst(Type *Ty, unsigned NumOps) :
    Instruction(Ty, RT_CallInst, NumOps) {}

void *CallInst::operator new(size_t s, unsigned n) {
  return User::operator new(s, n);
}

CallInst *CallInst::get(Value *Callee, std::vector<Value *> Ops) {
  auto NumOps = Ops.size();
  auto Obj = new (NumOps + 1) CallInst(Callee->getType(), NumOps);
  Obj->NumAllocatedOps = NumOps + 1;
  Obj->setOperand(-1, Callee);
  for (unsigned OpN = 0; OpN < NumOps; OpN++)
    Obj->setOperand(OpN, Ops[OpN]);
  return Obj;
}

bool CallInst::classof(const Value *V) {
  return V->getValID() == RT_CallInst;
}

Value *CallInst::getCallee() const {
  return getOperand(-1);
}

Value *CallInst::getOperand(int i) const {
  assert(i > -2 && "getOperand() below -1!");
  assert(i < (int)NumOperands && "getOperand() out of range!");
  return getOperandList()[i + 1];
}

void CallInst::setOperand(int i, Value *Val) {
  assert(i > -2 && "setOperand() below -1!");
  assert(i < (int)NumOperands && "setOperand() out of range!");
  getOperandList()[i + 1].set(Val);
}

void CallInst::print(std::ostream &Stream) const {
  Stream << getCallee()->getName() << " ~" << *VTy;
  for (auto O: getOperands())
    Stream << std::endl << *O;
}

MallocInst::MallocInst(std::string N, Type *Ty) :
    Instruction(Ty, RT_MallocInst, 1, N) {}

void *MallocInst::operator new(size_t s) {
  return User::operator new(s, 1);
}

MallocInst *MallocInst::get(std::string N, Value *V, Context *K) {
  auto Obj = new MallocInst(N, UnType::get(K));
  Obj->setOperand(0, V);
  return Obj;
}

bool MallocInst::classof(const Value *V) {
  return V->getValID() == RT_MallocInst;
}

void MallocInst::setVal(Value *V) {
  setOperand(0, V);
}

Value *MallocInst::getVal() {
  return getOperand(0);
}

void MallocInst::print(std::ostream &Stream) const {
  Stream << Name << " = " << *getOperand(0);
}

LoadInst::LoadInst(Value *V, std::string Name) :
    Instruction(V->getType(), RT_LoadInst, 1, Name) {
  setOperand(0, V);
}

void *LoadInst::operator new(size_t s) {
  return User::operator new (s, 1);
}

LoadInst *LoadInst::get(Value *V, std::string Name) {
  return new LoadInst(V, Name);
}

bool LoadInst::classof(const Value *V) {
  return V->getValID() == RT_LoadInst;
}

Value *LoadInst::getVal() const {
  return getOperand(0);
}

void LoadInst::print(std::ostream &Stream) const {
  Stream << Name << " ~" << *VTy;
}

ReturnInst::ReturnInst(Type *Ty, bool IsNotVoid) :
    Instruction(Ty, RT_ReturnInst, IsNotVoid) {}

void *ReturnInst::operator new(size_t s, unsigned NumberOfArgs) {
  return User::operator new(s, NumberOfArgs);
}

ReturnInst *ReturnInst::get(Value *V, Context *K) {
  if (!V)
    return new (0) ReturnInst(VoidType::get(K), false);
  auto Obj = new (1) ReturnInst(V->getType(), true);
  Obj->setOperand(0, V);
  return Obj;
}

bool ReturnInst::classof(const Value *V) {
  return V->getValID() == RT_ReturnInst;
}

void ReturnInst::setVal(Value *V) {
  setOperand(0, V);
}

Value *ReturnInst::getVal() {
  auto Operands = getOperands();
  if (Operands.size())
    return Operands[0];
  return nullptr;
}

void ReturnInst::print(std::ostream &Stream) const {
  Stream << "ret " << *getOperand(0);
}

IfInst::IfInst(Type *Ty):
    Instruction(Ty, RT_IfInst, 3) {}

void *IfInst::operator new(size_t s) {
  return User::operator new(s, 3);
}

IfInst *IfInst::get(Value *Conditional, BasicBlock *TrueBB,
                    BasicBlock *FalseBB, Context *K) {
  auto Obj = new IfInst(UnType::get(K));
  Obj->setOperand(0, Conditional);
  Obj->setOperand(1, TrueBB);
  Obj->setOperand(2, FalseBB);
  return Obj;
}

bool IfInst::classof(const Value *V) {
  return V->getValID() == RT_IfInst;
}

Value *IfInst::getConditional() const {
  return getOperand(0);
}

void IfInst::setConditional(Value *C)
{
  setOperand(0, C);
}

BasicBlock *IfInst::getTrueBB() const {
  return cast<BasicBlock>(getOperand(1));
}
BasicBlock *IfInst::getFalseBB() const {
  return cast<BasicBlock>(getOperand(2));
}

void IfInst::print(std::ostream &Stream) const {
  Stream << "if (" << *getConditional() << ") {" << std::endl;
  for (auto V: *getTrueBB())
    Stream << *V << std::endl;
  Stream << "} else {" << std::endl;
  for (auto V: *getFalseBB())
    Stream << *V << std::endl;
  Stream << "}";
}
}
