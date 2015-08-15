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
  Stream << "+ ~" << *getType();
  for (auto O: getOperands())
    Stream << std::endl << *O;
}

CallInst::CallInst(std::string FunctionName, Type *Ty, unsigned NumOps) :
    Instruction(Ty, RT_CallInst, NumOps), Callee(FunctionName) {}

void *CallInst::operator new(size_t s, unsigned n) {
  return User::operator new(s, n);
}

CallInst *CallInst::get(std::string FunctionName,
                        std::vector<Value *> Ops, Context *K) {
  auto NumOps = Ops.size();
  auto Obj = new (NumOps) CallInst(FunctionName, UnType::get(K), NumOps);
  for (unsigned OpN = 0; OpN < NumOps; OpN++)
    Obj->setOperand(OpN, Ops[OpN]);
  return Obj;
}

CallInst *CallInst::get(std::string FunctionName, Value *Op, Context *K) {
  std::vector<Value *> Ops = { Op };
  return get(FunctionName, Ops, K);
}

CallInst *CallInst::get(std::string FunctionName, Context *K) {
  std::vector<Value *> Ops;
  return get(FunctionName, Ops, K);
}


bool CallInst::classof(const Value *V) {
  return V->getValID() == RT_CallInst;
}

std::string CallInst::getCallee() {
  return Callee;
}

void CallInst::print(std::ostream &Stream) const {
  Stream << Callee << " ~" << *getType();
  for (auto O: getOperands())
    Stream << std::endl << *O;
}

MallocInst::MallocInst(std::string N, Type *Ty) :
    Instruction(Ty, RT_MallocInst, 1, N) {}

void *MallocInst::operator new(size_t s) {
  return User::operator new(s, 1);
}

MallocInst *MallocInst::get(std::string N, Value *V, Context *K) {
  auto Obj = new MallocInst(N, VoidType::get(K));
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
  return getOperands()[0];
}

void MallocInst::print(std::ostream &Stream) const {
  Stream << Name << " = " << *getOperands()[0];
}

LoadInst::LoadInst(std::string N, Type *T, RTValue ID) :
    Instruction(T, ID, 0, N) {}

void *LoadInst::operator new(size_t s) {
  return User::operator new (s);
}

LoadInst *LoadInst::get(std::string N, Type *T) {
  return new LoadInst(N, T);
}

bool LoadInst::classof(const Value *V) {
  return V->getValID() == RT_LoadInst;
}

void LoadInst::print(std::ostream &Stream) const {
  Stream << Name << " ~" << *getType();
}

IfInst::IfInst(Type *Ty, Value * Conditional_,
               BasicBlock *TrueBB_, BasicBlock *FalseBB_):
    Instruction(Ty, RT_IfInst, 3), Conditional(Conditional_),
    TrueBB(TrueBB_), FalseBB(FalseBB_) {}

void *IfInst::operator new(size_t s) {
  return User::operator new(s, 3);
}

IfInst *IfInst::get(Value * Conditional, BasicBlock *TrueBB,
                    BasicBlock *FalseBB, Context *K) {
  return new IfInst(UnType::get(K), Conditional,
                    TrueBB, FalseBB);
}

bool IfInst::classof(const Value *V) {
  return V->getValID() == RT_IfInst;
}

Value *IfInst::getConditional() {
  return Conditional;
}

void IfInst::setConditional(Value *C)
{
  Conditional = C;
}

BasicBlock *IfInst::getTrueBB() {
  return TrueBB;
}
BasicBlock *IfInst::getFalseBB() {
  return FalseBB;
}

void IfInst::print(std::ostream &Stream) const {
  Stream << "if (" << *Conditional << ") {" << std::endl;
  for (auto V: *TrueBB)
    Stream << *V << std::endl;
  Stream << "} else {" << std::endl;
  for (auto V: *FalseBB)
    Stream << *V << std::endl;
  Stream << "}";
}
}
