//-*- C++ -*-

#ifndef SYMBOLTABLE_H
#define SYMBOLTABLE_H

#include "llvm/IR/Value.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/ADT/FoldingSet.h"

#include <cstdlib>
#include <map>

#include "location.hh"
#include "rhine/Diagnostic.h"

namespace rhine {
class Context {
  // For function overloading, NameTypeMapping should map std::string to
  // std::vector<Symbol *>
  std::map <std::string, class Type *> NameTypeMapping;
  std::map <std::string, llvm::Value *> SymbolMapping;

public:
  llvm::BumpPtrAllocator RhAllocator;
  llvm::FoldingSet<class Symbol> SymbolCache;
  llvm::FoldingSet<class FunctionType> FTyCache;
  llvm::FoldingSet<class IntegerType> ITyCache;
  llvm::IRBuilder<> *Builder;
  DiagnosticPrinter *DiagPrinter;

  Context(std::ostream &ErrStream = std::cerr):
      Builder(new llvm::IRBuilder<>(llvm::getGlobalContext())),
      DiagPrinter(new DiagnosticPrinter(&ErrStream)) {}

  // The big free
  void releaseMemory() {
    RhAllocator.Reset();
    SymbolCache.clear();
    FTyCache.clear();
    ITyCache.clear();
    NameTypeMapping.clear();
    SymbolMapping.clear();
  }

  //===--------------------------------------------------------------------===//
  // Functions that operate on SymbolMapping.
  //===--------------------------------------------------------------------===//
  bool addMapping(std::string S, llvm::Value *V) {
    if (SymbolMapping.find(S) != SymbolMapping.end())
      return false;
    SymbolMapping.insert(std::make_pair(S, V));
    return true;
  }
  llvm::Value *getMapping(std::string S) {
    auto V = SymbolMapping.find(S);
    return V == SymbolMapping.end() ? nullptr : V->second;
  }
  llvm::Value *getMapping(std::string S, location SourceLoc) {
    auto V = SymbolMapping.find(S);
    if (V == SymbolMapping.end()) {
      DiagPrinter->errorReport(SourceLoc, "unbound variable " + S);
      exit(1);
    }
    return V->second;
  }
  //===--------------------------------------------------------------------===//
  // Functions that operate on NameTypeMapping.
  //===--------------------------------------------------------------------===//
  bool addNameTypeMapping(std::string N, class Type *T) {
    if (NameTypeMapping.find(N) != NameTypeMapping.end())
      return false;
    NameTypeMapping.insert(std::make_pair(N, T));
    return true;
  }
  Type *getNameTypeMapping(std::string S, location SourceLoc) {
    auto V = NameTypeMapping.find(S);
    if (V == NameTypeMapping.end()) {
      DiagPrinter->errorReport(SourceLoc, "untyped variable " + S);
      exit(1);
    }
    return V->second;
  }
};
}

#endif
