//-*- C++ -*-

#ifndef PARSEDRIVER_H
#define PARSEDRIVER_H

#include <iostream>
#include <string>
#include <vector>

namespace rhine {
class Context;
class PTree;

class ParseDriver
{
public:

  ParseDriver(class PTree &Tree, Context *SharedCtx, bool Debug = false);
  bool parseStream(std::istream &In,
                   const std::string &StreamName_ = "stream input");
  bool parseString(const std::string &Input,
                   const std::string &StreamName_ = "string stream");
  bool parseFile(const std::string &filename);

  bool TraceScanning;
  bool TraceParsing;
  class Lexer *Lexx;
  PTree &Root;
  Context *Ctx;
};
}

#endif
