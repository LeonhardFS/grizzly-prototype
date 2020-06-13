#ifndef C_CODE_COMPILER_HPP
#define C_CODE_COMPILER_HPP

#include <memory>
#include <string>
#include <vector>

namespace llvm {
class LLVMContext;
class ExecutionEngine;
} // namespace llvm

namespace clang {
class CompilerInstance;
}

class PipelineStage;
typedef std::shared_ptr<PipelineStage> PipelineStagePtr;

class SharedLibrary;
typedef std::shared_ptr<SharedLibrary> SharedLibraryPtr;

class SharedLibrary {
public:
  ~SharedLibrary();

  static SharedLibraryPtr load(const std::string &file_path);
  void *getSymbol(const std::string &mangeled_symbol_name) const;

  template <typename Function> Function getFunction(const std::string &mangeled_symbol_name) const {
    return reinterpret_cast<Function>(getSymbol(mangeled_symbol_name));
  }

private:
  SharedLibrary(void *shared_lib);
  void *shared_lib_;
};

class CompiledCCode {
public:
  virtual ~CompiledCCode() {}

  template <typename Function> Function getFunctionPointer(const std::string &name) {
    // INFO
    // http://www.trilithium.com/johan/2004/12/problem-with-dlsym/
    // No real solution in 2016.
    static_assert(sizeof(void *) == sizeof(Function), "Void pointer to function pointer conversion will not work!"
                                                      " If you encounter this, run!");

    union converter {
      void *v_ptr;
      Function f_ptr;
    };

    converter conv;
    conv.v_ptr = getFunctionPointerImpl(name);

    return conv.f_ptr;
  }

  double getCompileTimeInSeconds() const { return compile_time_in_ns_ / double(1e9); }

protected:
  CompiledCCode(long compile_time) : compile_time_in_ns_(compile_time) {}

  virtual void *getFunctionPointerImpl(const std::string &name) = 0;

private:
  long compile_time_in_ns_;
};

typedef std::shared_ptr<CompiledCCode> CompiledCCodePtr;

class CCodeCompiler {
public:
  CCodeCompiler();

  CompiledCCodePtr compile(const std::string &source, const std::string name);

private:
  void init();
  void initCompilerArgs();

  long createPrecompiledHeader();
  bool rebuildPrecompiledHeader();

  std::vector<std::string> getPrecompiledHeaderCompilerArgs();
  std::vector<std::string> getCompilerArgs();

  CompiledCCodePtr compileWithSystemCompiler(const std::string &source, const long pch_time, const std::string name);

  void callSystemCompiler(const std::vector<std::string> &args);

  CompiledCCodePtr compileWithJITCompiler(const std::string &source, const long pch_time);

  void initLLVM();

  void prepareClangCompiler(const std::string &source, const std::vector<const char *> &args,
                            clang::CompilerInstance &compiler);

  std::pair<std::shared_ptr<llvm::LLVMContext>, std::shared_ptr<llvm::ExecutionEngine>>
  createLLVMContextAndEngine(clang::CompilerInstance &compiler);

  std::vector<const char *> convertStringToCharPtrVec(const std::vector<std::string> &data);

  void handleDebugging(const std::string &source);

  bool use_clang_jit_ = false;
  bool show_generated_code_ = false;
  bool debug_code_generator_ = false;
  bool keep_last_generated_query_code_ = false;
  std::vector<std::string> compiler_args_;

  const static std::string IncludePath;
  const static std::string MinimalApiHeaderPath;
  std::string PrecompiledHeaderName;
};

void exportSourceToFile(const std::string &filename, const std::string &source);
void pretty_print_code(const std::string &source);

#endif // C_CODE_COMPILER_HPP
