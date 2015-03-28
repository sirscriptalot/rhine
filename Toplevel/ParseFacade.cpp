#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/Module.h"

#include "rhine/Ast.h"
#include "rhine/ParseDriver.h"

#include <iostream>
#include <string>

namespace rhine {
std::string LLToPP (llvm::Value *Obj)
{
  std::string Output;
  llvm::raw_string_ostream OutputStream(Output);
  Obj->print(OutputStream);
  return OutputStream.str();
}

llvm::Value *parsePrgString(std::string PrgString,
                            std::ostream &ErrStream,
                            bool Debug)
{
  auto Root = rhine::SExpr();
  auto Driver = rhine::ParseDriver(Root, ErrStream, Debug);
  auto M = new llvm::Module("main", RhContext);
  if (Driver.parseString(PrgString))
    return Root.Body.empty() ? Root.Defuns.back()->toLL(M) :
      Root.Body.back()->toLL(M);
  else
    return llvm::ConstantInt::get(RhContext, APInt(32, 0));
}

void parseFacade(std::string Filename, bool Debug) {
  auto Root = rhine::SExpr();
  auto Driver = rhine::ParseDriver(Root, std::cerr, Debug);
  auto M = new llvm::Module("main", RhContext);
  Driver.parseFile(Filename);
  for (auto ve : Root.Body)
    ve->toLL(M);
  for (auto ve : Root.Defuns)
    ve->toLL(M);
  M->dump();
}
}