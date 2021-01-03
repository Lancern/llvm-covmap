//
// Created by Sirui Mu on 2021/1/3.
//

#include <cassert>

#include <llvm/Pass.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Module.h>
#include <llvm/Support/Casting.h>
#include <llvm/Support/raw_ostream.h>

namespace llvm {

namespace covmap {

constexpr static const char *CoverageFunctionName = "__llvm_covmap_hit_function";

static llvm::FunctionType *GetCoverageFunctionType(llvm::LLVMContext &context) noexcept {
  auto uint64Type = llvm::IntegerType::get(context, 64);
  llvm::Type *argTypes[2] = { uint64Type, uint64Type };
  auto voidType = llvm::Type::getVoidTy(context);
  return llvm::FunctionType::get(voidType, argTypes, false);
}

class CoverageMapPass : public llvm::ModulePass {
public:
  static char ID;

  explicit CoverageMapPass() noexcept
    : llvm::ModulePass { ID }
  { }

  bool runOnModule(llvm::Module &module) final {
    llvm::outs() << "========== LLVM Coverage Map Instrumentation Pass ==========\n";

    uint64_t numFunctions = 0;
    for (const auto &function : module) {
      if (!function.isDeclaration()) {
        ++numFunctions;
      }
    }

    llvm::outs() << "Number of functions found: " << numFunctions << "\n";

    if (numFunctions == 0) {
      return false;
    }

    auto numFunctionsConstantValue = llvm::ConstantInt::get(
        llvm::IntegerType::get(module.getContext(), 64),
        numFunctions);

    auto coverageFunctionCallee = module.getOrInsertFunction(
        CoverageFunctionName,
        GetCoverageFunctionType(module.getContext()));

    uint64_t functionId = 0;
    for (auto &function : module) {
      if (function.isDeclaration()) {
        continue;
      }

      auto &entryBlock = function.getEntryBlock();

      auto functionIdConstantValue = llvm::ConstantInt::get(
          llvm::IntegerType::get(module.getContext(), 64),
          functionId);
      llvm::Value *callArgs[2] = { numFunctionsConstantValue, functionIdConstantValue };
      llvm::CallInst::Create(coverageFunctionCallee, callArgs)
          ->insertBefore(&*entryBlock.getFirstInsertionPt());

      ++functionId;
    }

    assert(functionId == numFunctions && "Wrong number of functions");

    return true;
  }
};

char CoverageMapPass::ID = 0;

__attribute__((unused))
static llvm::RegisterPass<CoverageMapPass> RegisterCoverageMapPass { // NOLINT(cert-err58-cpp)
  "covmap",
  "Collect coverage bitmap",
  false,
  false
};

} // namespace covmap

} // namespace llvm
