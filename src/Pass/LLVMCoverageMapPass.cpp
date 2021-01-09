//
// Created by Sirui Mu on 2021/1/3.
//

#include <cerrno>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <random>

#include <llvm/Pass.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/Module.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Transforms/IPO/PassManagerBuilder.h>

namespace llvm {

namespace covmap {

constexpr static const char *CoverageFunctionName = "__llvm_covmap_hit_function";

constexpr static const uint32_t DefaultInstrumentationRatio = 100;

static llvm::FunctionType *GetCoverageFunctionType(llvm::LLVMContext &context) noexcept {
  auto uint64Type = llvm::IntegerType::get(context, 64);
  llvm::Type *argTypes[1] = { uint64Type };
  auto voidType = llvm::Type::getVoidTy(context);
  return llvm::FunctionType::get(voidType, argTypes, false);
}

static unsigned GetInstrumentationRatio() noexcept {
  auto ratioStr = getenv("LLVM_COVMAP_INST_RATIO");
  if (!ratioStr) {
    return DefaultInstrumentationRatio;
  }

  errno = 0;
  uint32_t ratio = std::strtoul(ratioStr, nullptr, 10);
  if (errno != 0) {
    return DefaultInstrumentationRatio;
  }

  return ratio;
}

class CoverageMapPass : public llvm::ModulePass {
public:
  static char ID;

  explicit CoverageMapPass() noexcept
    : llvm::ModulePass { ID },
      _rnd(std::chrono::high_resolution_clock::now().time_since_epoch().count())
  { }

  bool runOnModule(llvm::Module &module) final {
    if (module.getFunction(CoverageFunctionName)) {
      // Already instrumented.
      return false;
    }

    auto coverageFunctionType = GetCoverageFunctionType(module.getContext());
    auto coverageFunctionCallee = module.getOrInsertFunction(CoverageFunctionName, coverageFunctionType);

    auto ratio = GetInstrumentationRatio();

    for (auto &function : module) {
      if (function.isDeclaration()) {
        continue;
      }

      if (!Probability(ratio)) {
        continue;
      }

      auto functionId = Random();
      auto &entryBlock = function.getEntryBlock();
      IRBuilder<> builder { &entryBlock, entryBlock.begin() };

      llvm::Value *callArgs[1] = { builder.getInt64(functionId) };
      builder.CreateCall(coverageFunctionCallee, callArgs);
    }

    return true;
  }

private:
  std::mt19937_64 _rnd;

  template <typename T>
  T Random(T min, T max) noexcept {
    std::uniform_int_distribution<T> dist { min, max };
    return dist(_rnd);
  }

  uint64_t Random() noexcept {
    return _rnd();
  }

  bool Probability(uint32_t ratio) noexcept {
    return Random<uint32_t>(0, 100) <= ratio;
  }
};

char CoverageMapPass::ID = 0;

static void RegisterCoverageMapPass(const llvm::PassManagerBuilder &, llvm::legacy::PassManagerBase &manager) noexcept {
  manager.add(new CoverageMapPass());
}

__attribute__((unused))
static llvm::RegisterPass<CoverageMapPass> RegisterCoverageMapLoadablePass { // NOLINT(cert-err58-cpp)
  "covmap",
  "Collect coverage bitmap",
  false,
  false
};

__attribute__((unused))
static llvm::RegisterStandardPasses RegisterCoverageMapStandardPass { // NOLINT(cert-err58-cpp)
    llvm::PassManagerBuilder::EP_ModuleOptimizerEarly,
    RegisterCoverageMapPass
};

__attribute__((unused))
static llvm::RegisterStandardPasses RegisterCoverageMapStandardPassOpt0 { // NOLINT(cert-err58-cpp)
  llvm::PassManagerBuilder::EP_EnabledOnOptLevel0,
  RegisterCoverageMapPass
};

} // namespace covmap

} // namespace llvm
