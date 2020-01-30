#ifndef GOOGLE_PROTOBUF_COMPILER_YAML_GENERATOR_H__
#define GOOGLE_PROTOBUF_COMPILER_YAML_GENERATOR_H__

#include <string>

#include <google/protobuf/compiler/code_generator.h>

namespace google {
namespace protobuf {
namespace compiler {
namespace yaml {

class LIBPROTOC_EXPORT Generator
    : public google::protobuf::compiler::CodeGenerator {
  virtual bool Generate(
      const FileDescriptor* file,
      const string& parameter,
      GeneratorContext* generator_context,
      string* error) const;
};

}  // namespace yaml
}  // namespace compiler
}  // namespace protobuf
}  // namespace google

#endif  // GOOGLE_PROTOBUF_COMPILER_YAML_GENERATOR_H__

