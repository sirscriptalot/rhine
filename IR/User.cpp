#include "rhine/IR/User.h"
#include "rhine/IR/Constant.h"

namespace rhine {
User::User(Type *Ty, RTValue ID, unsigned NumOps, std::string N) :
    Value(Ty, ID, N), NumOperands(NumOps), NumAllocatedOps(NumOps) {}

void *User::operator new(size_t Size, unsigned Us) {
  void *Storage = ::operator new (Us * sizeof(Use) + Size);
  auto Start = static_cast<Use *>(Storage);
  auto End = Start + Us;
  for (unsigned Iter = 0; Iter < Us; Iter++) {
    new (Start + Iter) Use(Iter);
  }
  auto Obj = reinterpret_cast<User *>(End);
  return Obj;
}

void *User::operator new(size_t Size) {
  return ::operator new (Size);
}

void User::operator delete(void *Usr) {
  User *Obj = static_cast<User *>(Usr);
  Use *Storage = static_cast<Use *>(Usr) - Obj->NumAllocatedOps;
  Use::zap(Storage, Storage + Obj->NumAllocatedOps, /* Delete */ false);
  ::operator delete(Storage);
}

bool User::classof(const Value *V) {
  return V->getValID() >= RT_User &&
    V->getValID() <= RT_IfInst;
}

Use *User::getOperandList() {
  return reinterpret_cast<Use *>(this) - NumAllocatedOps;
}

const Use *User::getOperandList() const {
  return const_cast<User *>(this)->getOperandList();
}

Value *User::getOperand(int i) const {
  assert(i < (int)NumOperands && "getOperand() out of range!");
  return getOperandList()[i];
}

void User::setOperand(int i, Value *Val) {
  assert(i < (int)NumOperands && "setOperand() out of range!");
  assert(!isa<Constant>(cast<Value>(this)) &&
         "Cannot mutate a constant with setOperand!");
  getOperandList()[i].set(Val);
}
}
