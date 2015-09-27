#include "rhine/Util/TestUtil.h"
#include "gtest/gtest.h"

using namespace rhine;

TEST(DollarOperator, WithFunction) {
  std::string SourcePrg =
    "def foo(x ~Int) do\n"
    "  ret '2';\n"
    "end\n"
    "def main() do\n"
    "  print $ foo 3;\n"
    "end";
  std::string ExpectedOut = "2";
  EXPECT_OUTPUT(SourcePrg, ExpectedOut);
}

TEST(CodeGen, WithRet)
{
  std::string SourcePrg =
    "def foo do\n"
    "  ret $ 3 + 2;\n"
    "end";
  std::string ExpectedLL =
    "define i32 @foo() gc \"rhgc\" {\n"
    "entry:\n"
    "  ret i32 5\n"
    "}\n";
  EXPECT_LL(SourcePrg, ExpectedLL);
}