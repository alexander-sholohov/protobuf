//
// YAML generator
// Alexander Sholohov <ra9yer@yahoo.com>
//

#include <sstream>

#include <google/protobuf/compiler/code_generator.h>
#include <google/protobuf/compiler/plugin.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/descriptor.pb.h>
#include <google/protobuf/io/printer.h>
#include <google/protobuf/io/zero_copy_stream.h>

#include <google/protobuf/compiler/yaml/yaml_generator.h>


namespace google {
namespace protobuf {
namespace compiler {
namespace yaml {

// Forward decls.
std::string IntToString(int32 value);
std::string StripDotProto(const std::string& proto_file);
std::string LabelForField(google::protobuf::FieldDescriptor* field);
std::string TypeName(google::protobuf::FieldDescriptor* field);
void GenerateMessage(const google::protobuf::Descriptor* message,
                     google::protobuf::io::Printer* printer);
void GenerateEnum(const google::protobuf::EnumDescriptor* en,
                  google::protobuf::io::Printer* printer);
void GenerateMessageAssignment(
    const std::string& prefix,
    const google::protobuf::Descriptor* message,
    google::protobuf::io::Printer* printer);
void GenerateEnumAssignment(
    const std::string& prefix,
    const google::protobuf::EnumDescriptor* en,
    google::protobuf::io::Printer* printer);

std::string IntToString(int32 value) {
  std::ostringstream os;
  os << value;
  return os.str();
}

std::string StripDotProto(const std::string& proto_file) {
  int lastindex = proto_file.find_last_of(".");
  return proto_file.substr(0, lastindex);
}

std::string GetOutputFilename(const std::string& proto_file) {
  return StripDotProto(proto_file) + ".yaml";
}

std::string LabelForField(const google::protobuf::FieldDescriptor* field) {
  switch (field->label()) {
    case FieldDescriptor::LABEL_OPTIONAL: return "optional";
    case FieldDescriptor::LABEL_REQUIRED: return "required";
    case FieldDescriptor::LABEL_REPEATED: return "repeated";
    default: assert(false); return "";
  }
}

std::string TypeName(const google::protobuf::FieldDescriptor* field) {
  switch (field->type()) {
    case FieldDescriptor::TYPE_INT32: return "int32";
    case FieldDescriptor::TYPE_INT64: return "int64";
    case FieldDescriptor::TYPE_UINT32: return "uint32";
    case FieldDescriptor::TYPE_UINT64: return "uint64";
    case FieldDescriptor::TYPE_SINT32: return "sint32";
    case FieldDescriptor::TYPE_SINT64: return "sint64";
    case FieldDescriptor::TYPE_FIXED32: return "fixed32";
    case FieldDescriptor::TYPE_FIXED64: return "fixed64";
    case FieldDescriptor::TYPE_SFIXED32: return "sfixed32";
    case FieldDescriptor::TYPE_SFIXED64: return "sfixed64";
    case FieldDescriptor::TYPE_DOUBLE: return "double";
    case FieldDescriptor::TYPE_FLOAT: return "float";
    case FieldDescriptor::TYPE_BOOL: return "bool";
    case FieldDescriptor::TYPE_ENUM: return "enum";
    case FieldDescriptor::TYPE_STRING: return "string";
    case FieldDescriptor::TYPE_BYTES: return "bytes";
    case FieldDescriptor::TYPE_MESSAGE: return "message";
    case FieldDescriptor::TYPE_GROUP: return "group";
    default: assert(false); return "";
  }
}

void GenerateField(const google::protobuf::FieldDescriptor* field,
                   google::protobuf::io::Printer* printer) 
{
  
  printer->Indent();
  if (field->is_map()) {
    printer->Print(" - name: $name$\n", "name", field->name());
    printer->Print("   fieldtype: map\n");
  } else {
    printer->Print(" - name: $name$\n", "name", field->name());
    printer->Print("   fieldopt: $fieldopt$\n", "fieldopt", LabelForField(field));
    printer->Print("   fieldtype: $fieldtype$\n", "fieldtype", TypeName(field));
    printer->Print("   number: $number$\n", "number", IntToString(field->number()));

    if (field->cpp_type() == FieldDescriptor::CPPTYPE_MESSAGE) {
      printer->Print("   subtype: $subtype$\n", "subtype", field->message_type()->full_name());
    } else if (field->cpp_type() == FieldDescriptor::CPPTYPE_ENUM) {
      printer->Print("   subtype: $subtype$\n", "subtype", field->enum_type()->full_name());
    }
  }
  printer->Outdent();
}


void GenerateMessage(const google::protobuf::Descriptor* message,
                     google::protobuf::io::Printer* printer) {

  // Don't generate MapEntry messages -- we use the Ruby extension's native
  // support for map fields instead.
  if (message->options().map_entry()) {
    return;
  }

  //printer->Print("---\n");

  printer->Print("- name: $name$\n", "name", message->full_name() );
  //printer->Print("shortname: $shortname$\n", "shortname", message->name() );
  printer->Print("  typedef: message\n" );
  printer->Print("  fields:\n" );
  for (int i = 0; i < message->field_count(); i++) {
    const FieldDescriptor* field = message->field(i);
    if (!field->containing_oneof()) {
      GenerateField(field, printer);
    }
  }


  for (int i = 0; i < message->enum_type_count(); i++) {
    GenerateEnum(message->enum_type(i), printer);
  }

}

void GenerateEnum(const google::protobuf::EnumDescriptor* en,
                  google::protobuf::io::Printer* printer) 
{

  //printer->Print("---\n");

  printer->Print("- name: $name$\n", "name", en->full_name() );
  //printer->Print("shortname: $shortname$\n", "shortname", en->name() );
  printer->Print("  typedef: enum\n" );
  if (en->containing_type() ) {
    printer->Print("  owner_type: $owner_type$\n", "owner_type", en->containing_type()->full_name() );
  }
  printer->Print("  options:\n" );

  for (int i = 0; i < en->value_count(); i++) {
    const EnumValueDescriptor* value = en->value(i);
    printer->Print("   - name: $name$\n", "name", value->name());
    printer->Print("     value: $value$\n", "value", IntToString(value->number()));
  }

}


bool UsesTypeFromFile(const Descriptor* message, const FileDescriptor* file,
                      string* error) {
  for (int i = 0; i < message->field_count(); i++) {
    const FieldDescriptor* field = message->field(i);
    if ((field->cpp_type() == FieldDescriptor::CPPTYPE_MESSAGE &&
         field->message_type()->file() == file) ||
        (field->type() == FieldDescriptor::TYPE_ENUM &&
         field->enum_type()->file() == file)) {
      *error = "proto3 message field " + field->full_name() + " in file " +
               file->name() + " has a dependency on a type from proto2 file " +
               file->name() +
               ".  Ruby doesn't support proto2 yet, so we must fail.";
      return true;
    }
  }

  for (int i = 0; i < message->nested_type_count(); i++) {
    if (UsesTypeFromFile(message->nested_type(i), file, error)) {
      return true;
    }
  }

  return false;
}


bool GenerateFile(const FileDescriptor* file, io::Printer* printer,
                  string* error) {

  printer->Print(
    "# Generated by the protocol buffer compiler.  DO NOT EDIT!\n"
    "# source: $filename$\n"
    "# package: $package$\n"
    "#\n",
    "filename", file->name(),
    "package", file->package() );


#if 0
  printer->Print(
    "require 'google/protobuf'\n\n");

  for (int i = 0; i < file->dependency_count(); i++) {
    if (!MaybeEmitDependency(file->dependency(i), file, printer, error)) {
      return false;
    }
  }

  printer->Print(
    "Google::Protobuf::DescriptorPool.generated_pool.build do\n");
  printer->Indent();
#endif
  for (int i = 0; i < file->message_type_count(); i++) {
    GenerateMessage(file->message_type(i), printer);
  }
  for (int i = 0; i < file->enum_type_count(); i++) {
    GenerateEnum(file->enum_type(i), printer);
  }
  return true;
}

bool Generator::Generate(
    const FileDescriptor* file,
    const string& parameter,
    GeneratorContext* generator_context,
    string* error) const {

    std::unique_ptr<io::ZeroCopyOutputStream> output(
      generator_context->Open(GetOutputFilename(file->name())));
  io::Printer printer(output.get(), '$');

  return GenerateFile(file, &printer, error);
}

}  // namespace yaml
}  // namespace compiler
}  // namespace protobuf
}  // namespace google
