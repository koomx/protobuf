// Protocol Buffers - Google's data interchange format
// Copyright 2008 Google Inc.  All rights reserved.
// https://developers.google.com/protocol-buffers/
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//     * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

// Author: kenton@google.com (Kenton Varda)
//  Based on original Protocol Buffers design by
//  Sanjay Ghemawat, Jeff Dean, and others.

#include <shark/compiler/cpp/enum.h>

#include <cstdint>
#include <limits>
#include <map>

#include <shark/compiler/cpp/helpers.h>
#include <shark/compiler/cpp/names.h>
#include <shark/io/printer.h>
#include <shark/stubs/strutil.h>

namespace shark::compiler::cpp {

    namespace {
        // The GOOGLE_ARRAYSIZE constant is the max enum value plus 1. If the max enum value
        // is kint32max, GOOGLE_ARRAYSIZE will overflow. In such cases we should omit the
        // generation of the GOOGLE_ARRAYSIZE constant.
        bool ShouldGenerateArraySize(const EnumDescriptor* descriptor) {
            int32_t max_value = descriptor->value(0)->number();
            for (int i = 0; i < descriptor->value_count(); i++) {
                if (descriptor->value(i)->number() > max_value) {
                    max_value = descriptor->value(i)->number();
                }
            }
            return max_value != std::numeric_limits<int32_t>::max();
        }

        // Returns the number of unique numeric enum values. This is less than
        // descriptor->value_count() when there are aliased values.
        int CountUniqueValues(const EnumDescriptor* descriptor) {
            std::set<int> values;
            for (int i = 0; i < descriptor->value_count(); ++i) {
                values.insert(descriptor->value(i)->number());
            }
            return values.size();
        }

    } // namespace

    EnumGenerator::EnumGenerator(const EnumDescriptor* descriptor,
        const std::map<std::string, std::string>& vars,
        const Options& options)
        : descriptor_(descriptor)
        , classname_(ClassName(descriptor, false))
        , options_(options)
        , generate_array_size_(ShouldGenerateArraySize(descriptor))
        , variables_(vars) {
        variables_["classname"] = classname_;
        variables_["classtype"] = QualifiedClassName(descriptor_, options);
        variables_["short_name"] = descriptor_->name();
        variables_["nested_name"] = descriptor_->name();
        variables_["resolved_name"] = ResolveKeyword(descriptor_->name());
        variables_["prefix"] = (descriptor_->containing_type() == nullptr) ? "" : classname_ + "_";
        variables_["underlying"] = EnumUnderlyingCppType(descriptor_);
    }

    EnumGenerator::~EnumGenerator() { }

    namespace {

        std::string FitsIntExpression(const EnumDescriptor* descriptor) {
            switch (descriptor->options().underlying_type()) {
            case EnumUnderlyingType::ENUM_UNDERLYING_TYPE_INT8:
                return "value >= -128 && value <= 127";
            case EnumUnderlyingType::ENUM_UNDERLYING_TYPE_UINT8:
                return "value >= 0 && value <= 255";
            case EnumUnderlyingType::ENUM_UNDERLYING_TYPE_INT16:
                return "value >= -32768 && value <= 32767";
            case EnumUnderlyingType::ENUM_UNDERLYING_TYPE_UINT16:
                return "value >= 0 && value <= 65535";
            case EnumUnderlyingType::ENUM_UNDERLYING_TYPE_INT32:
            case EnumUnderlyingType::ENUM_UNDERLYING_TYPE_UINT32:
                return "true";
            default:
                return "true";
            }
        }

        std::string FitsWireExpression(const EnumDescriptor* descriptor,
            const std::string& classname) {
            switch (descriptor->options().underlying_type()) {
            case EnumUnderlyingType::ENUM_UNDERLYING_TYPE_INT8:
            case EnumUnderlyingType::ENUM_UNDERLYING_TYPE_INT16:
            case EnumUnderlyingType::ENUM_UNDERLYING_TYPE_INT32:
                return "(value <= 2147483647u || (value >> 32) == 0xffffffffu) && " + classname + "_FitsUnderlying(static_cast<int>(value))";
            case EnumUnderlyingType::ENUM_UNDERLYING_TYPE_UINT8:
                return "value <= 255u";
            case EnumUnderlyingType::ENUM_UNDERLYING_TYPE_UINT16:
                return "value <= 65535u";
            case EnumUnderlyingType::ENUM_UNDERLYING_TYPE_UINT32:
                return "value <= 0xffffffffu";
            case EnumUnderlyingType::ENUM_UNDERLYING_TYPE_UNSPECIFIED:
            default:
                return "(value <= 2147483647u || (value >> 32) == 0xffffffffu) && " + classname + "_FitsUnderlying(static_cast<int>(value))";
            }
        }

    } // namespace

    void EnumGenerator::GenerateDefinition(io::Printer* printer) {
        Formatter format(printer, variables_);
        // "enum" and "enum class" are equivalent: always generate a C++ enum
        // class with the selected underlying type.
        format("enum class ${1$$classname$$}$ : $underlying$ {\n", descriptor_);
        format.Indent();

        min_value_ = descriptor_->value(0);
        max_value_ = descriptor_->value(0);

        for (int i = 0; i < descriptor_->value_count(); i++) {
            auto format_value = format;
            format_value.

            Set("name", EnumValueName(descriptor_->value(i)));
            // In C++, an value of -2147483648 gets interpreted as the negative of
            // 2147483648, and since 2147483648 can't fit in an integer, this produces a
            // compiler warning.  This works around that issue.
            format_value.Set("number", Int32ToString(descriptor_->value(i)->number()));
            format_value.Set("deprecation",
                DeprecatedAttribute(options_, descriptor_->value(i)));

            if (i > 0)
                format_value(",\n");
            format_value("${1$$name$$}$ $deprecation$= $number$",
                descriptor_->value(i));

            if (descriptor_->value(i)->number() < min_value_->number()) {
                min_value_ = descriptor_->value(i);
            }
            if (descriptor_->value(i)->number() > max_value_->number()) {
                max_value_ = descriptor_->value(i);
            }
        }

        format.Outdent();
        format("\n};\n");

        {
            format(
                "constexpr $underlying$ $classname$_to_underlying($classname$ value) {\n"
                "  return static_cast<$underlying$>(value);\n"
                "}\n"
                "inline bool $classname$_FitsUnderlying(int value) {\n"
                "  return $1$;\n"
                "}\n"
                "inline bool $classname$_FitsWire(::uint64_t value) {\n"
                "  return $2$;\n"
                "}\n",
                FitsIntExpression(descriptor_),
                FitsWireExpression(descriptor_, classname_));
            if (descriptor_->options().enable_operator()) {
                format(
                    "constexpr $classname$ operator|($classname$ lhs, $classname$ rhs) {\n"
                    "  return static_cast<$classname$>(static_cast<$underlying$>("
                    "$classname$_to_underlying(lhs) | $classname$_to_underlying(rhs)));\n"
                    "}\n"
                    "constexpr $classname$ operator&($classname$ lhs, $classname$ rhs) {\n"
                    "  return static_cast<$classname$>(static_cast<$underlying$>("
                    "$classname$_to_underlying(lhs) & $classname$_to_underlying(rhs)));\n"
                    "}\n"
                    "constexpr $classname$ operator^($classname$ lhs, $classname$ rhs) {\n"
                    "  return static_cast<$classname$>(static_cast<$underlying$>("
                    "$classname$_to_underlying(lhs) ^ $classname$_to_underlying(rhs)));\n"
                    "}\n"
                    "constexpr $classname$ operator~($classname$ value) {\n"
                    "  return static_cast<$classname$>(static_cast<$underlying$>("
                    "~$classname$_to_underlying(value)));\n"
                    "}\n"
                    "constexpr $classname$& operator|=($classname$& lhs, $classname$ rhs) {\n"
                    "  return lhs = lhs | rhs;\n"
                    "}\n"
                    "constexpr $classname$& operator&=($classname$& lhs, $classname$ rhs) {\n"
                    "  return lhs = lhs & rhs;\n"
                    "}\n"
                    "constexpr $classname$& operator^=($classname$& lhs, $classname$ rhs) {\n"
                    "  return lhs = lhs ^ rhs;\n"
                    "}\n");
            }
        }
        format(
            "$dllexport_decl $bool $classname$_IsValid(int value);\n");
        /*
        format(
            "$dllexport_decl $bool $classname$_IsValid(int value);\n"
            "constexpr $classname$ ${1$$prefix$$short_name$_MIN$}$ = "
            "$classname$::$2$;\n"
            "constexpr $classname$ ${1$$prefix$$short_name$_MAX$}$ = "
            "$classname$::$3$;\n",
            descriptor_, EnumValueName(min_value_), EnumValueName(max_value_));

        if (generate_array_size_) {
            format(
                "constexpr int ${1$$prefix$$short_name$_ARRAYSIZE$}$ = "
                "$classname$_to_underlying($prefix$$short_name$_MAX) + 1;\n\n",
                descriptor_);
        }*/

        if (HasDescriptorMethods(descriptor_->file(), options_)) {
            format(
                "$dllexport_decl $const ::$proto_ns$::EnumDescriptor* "
                "$classname$_descriptor();\n");
        }

        // The _Name and _Parse functions. The lite implementation is table-based, so
        // we make sure to keep the tables hidden in the .cc file.
        if (!HasDescriptorMethods(descriptor_->file(), options_)) {
            format("const std::string& $classname$_Name($classname$ value);\n");
        }
        // The _Name() function accepts the enum type itself but also any integral
        // type.
        format(
            "template<typename T>\n"
            "inline const std::string& $classname$_Name(T enum_t_value) {\n"
            "  static_assert(::std::is_same<T, $classname$>::value ||\n"
            "    ::std::is_integral<T>::value,\n"
            "    \"Incorrect type passed to function $classname$_Name.\");\n");
        if (HasDescriptorMethods(descriptor_->file(), options_)) {
            format(
                "  return ::$proto_ns$::internal::NameOfEnum(\n"
                "    $classname$_descriptor(),\n"
                "    $classname$_to_underlying(static_cast<$classname$>(enum_t_value)));\n");
        } else {
            format(
                "  return $classname$_Name(static_cast<$classname$>(enum_t_value));\n");
        }
        format("}\n");

        if (HasDescriptorMethods(descriptor_->file(), options_)) {
            format(
                "inline bool $classname$_Parse(\n"
                "    ::SHARK_NAMESPACE_ID::ConstStringParam name, $classname$* "
                "value) "
                "{\n"
                "  return ::$proto_ns$::internal::ParseNamedEnum<$classname$>(\n"
                "    $classname$_descriptor(), name, value);\n"
                "}\n");
        } else {
            format(
                "bool $classname$_Parse(\n"
                "    ::SHARK_NAMESPACE_ID::ConstStringParam name, $classname$* "
                "value);\n");
        }
    }

    void EnumGenerator::GenerateGetEnumDescriptorSpecializations(
        io::Printer* printer) {

        bool has_des = HasDescriptorMethods(descriptor_->file(), options_);
        Formatter format(printer, variables_);
        format.Indent();
        format("template <>\n");
        format("struct EnumTraitImpl<$classtype$> {\n");
        format.Indent();
        format("using underlying = $underlying$;\n");
        format("static constexpr $classtype$ enum_min = $classtype$::$1$;\n", EnumValueName(min_value_));
        format("static constexpr $classtype$ enum_max = $classtype$::$1$;\n", EnumValueName(max_value_));
        format("static constexpr uint32_t value_count = $1$;\n", descriptor_->value_count());
        format("static constexpr bool has_descriptor = $1$;\n", (has_des ? "true" : "false"));

        if (has_des) {
            format("static inline const EnumDescriptor* descriptor() {\n");
            format.Indent();
            format("return $classtype$_descriptor();\n");
            format.Outdent();
            format("}\n");
        }

        format("static inline const std::string& enum_name($classtype$ e) {\n");
        format.Indent();
        format("return $classtype$_Name(e);\n");
        format.Outdent();
        format("}\n");

        format("static inline bool enum_parse(::SHARK_NAMESPACE_ID::ConstStringParam name, $classtype$ *e) {\n");
        format.Indent();
        format("return $classtype$_Parse(name, e);\n");
        format.Outdent();
        format("}\n");

        format("static inline bool is_valid(int64_t value) {\n");
        format.Indent();
        format("return $classtype$_IsValid(value);\n");
        format.Outdent();
        format("}\n");

        format.Outdent();
        format("};\n\n");

        format("template <> struct IsSharkEnum< $classtype$> : ::std::true_type {};\n");

        format.Outdent();
    }

    void EnumGenerator::GenerateSymbolImports(io::Printer* printer) const {
        Formatter format(printer, variables_);
        format("typedef $classname$ $resolved_name$;\n");
        format(
            "static inline bool $nested_name$_IsValid(int value) {\n"
            "  return $classname$_IsValid(value);\n"
            "}\n");
        /*
        format(
            "static inline bool $nested_name$_IsValid(int value) {\n"
            "  return $classname$_IsValid(value);\n"
            "}\n"
            "static constexpr $resolved_name$ ${1$$nested_name$_MIN$}$ =\n"
            "  $classname$_$nested_name$_MIN;\n"
            "static constexpr $resolved_name$ ${1$$nested_name$_MAX$}$ =\n"
            "  $classname$_$nested_name$_MAX;\n",
            descriptor_);
        if (generate_array_size_) {
            format(
                "static constexpr int ${1$$nested_name$_ARRAYSIZE$}$ =\n"
                "  $classname$_$nested_name$_ARRAYSIZE;\n",
                descriptor_);
        }
        */
        if (HasDescriptorMethods(descriptor_->file(), options_)) {
            format(
                "static inline const ::$proto_ns$::EnumDescriptor*\n"
                "$nested_name$_descriptor() {\n"
                "  return $classname$_descriptor();\n"
                "}\n");
        }

        format(
            "template<typename T>\n"
            "static inline const std::string& $nested_name$_Name(T enum_t_value) {\n"
            "  static_assert(::std::is_same<T, $resolved_name$>::value ||\n"
            "    ::std::is_integral<T>::value,\n"
            "    \"Incorrect type passed to function $nested_name$_Name.\");\n"
            "  return $classname$_Name(enum_t_value);\n"
            "}\n");
        format(
            "static inline bool "
            "$nested_name$_Parse(::SHARK_NAMESPACE_ID::ConstStringParam name,\n"
            "    $resolved_name$* value) {\n"
            "  return $classname$_Parse(name, value);\n"
            "}\n");
    }

    void EnumGenerator::GenerateMethods(int idx, io::Printer* printer) {
        Formatter format(printer, variables_);
        if (HasDescriptorMethods(descriptor_->file(), options_)) {

            format(
                "const ::$proto_ns$::EnumDescriptor* $classname$_descriptor() {\n"
                "  ::$proto_ns$::internal::AssignDescriptors(&$desc_table$);\n"
                "  return $file_level_enum_descriptors$[$1$];\n"
                "}\n",
                idx);
        }

        format(
            "bool $classname$_IsValid(int value) {\n"
            "  switch (value) {\n");

        // Multiple values may have the same number.  Make sure we only cover
        // each number once by first constructing a set containing all valid
        // numbers, then printing a case statement for each element.

        std::set<int> numbers;
        for (int j = 0; j < descriptor_->value_count(); j++) {
            const EnumValueDescriptor* value = descriptor_->value(j);
            numbers.insert(value->number());
        }

        for (std::set<int>::iterator iter = numbers.begin(); iter != numbers.end();
            ++iter) {
            format("    case $1$:\n", Int32ToString(*iter));
        }

        format(
            "      return true;\n"
            "    default:\n"
            "      return false;\n"
            "  }\n"
            "}\n"
            "\n");

        if (!HasDescriptorMethods(descriptor_->file(), options_)) {
            // In lite mode (where descriptors are unavailable), we generate separate
            // tables for mapping between enum names and numbers. The _entries table
            // contains the bulk of the data and is sorted by name, while
            // _entries_by_number is sorted by number and just contains pointers into
            // _entries. The two tables allow mapping from name to number and number to
            // name, both in time logarithmic in the number of enum entries. This could
            // probably be made faster, but for now the tables are intended to be simple
            // and compact.
            //
            // Enums with allow_alias = true support multiple entries with the same
            // numerical value. In cases where there are multiple names for the same
            // number, we treat the first name appearing in the .shark file as the
            // canonical one.
            std::map<std::string, int> name_to_number;
            std::map<int, std::string> number_to_canonical_name;
            for (int i = 0; i < descriptor_->value_count(); i++) {
                const EnumValueDescriptor* value = descriptor_->value(i);
                name_to_number.emplace(value->name(), value->number());
                // The same number may appear with multiple names, so we use emplace() to
                // let the first name win.
                number_to_canonical_name.emplace(value->number(), value->name());
            }

            format(
                "static ::$proto_ns$::internal::ExplicitlyConstructed<std::string> "
                "$classname$_strings[$1$] = {};\n\n",
                CountUniqueValues(descriptor_));

            // We concatenate all the names for a given enum into one big string
            // literal. If instead we store an array of string literals, the linker
            // seems to put all enum strings for a given .shark file in the same
            // section, which hinders its ability to strip out unused strings.
            format("static const char $classname$_names[] =");
            for (const auto& p : name_to_number) {
                format("\n  \"$1$\"", p.first);
            }
            format(";\n\n");

            format(
                "static const ::$proto_ns$::internal::EnumEntry $classname$_entries[] "
                "= {\n");
            int i = 0;
            std::map<int, int> number_to_index;
            int data_index = 0;
            for (const auto& p : name_to_number) {
                format("  { {$classname$_names + $1$, $2$}, $3$ },\n", data_index,
                    p.first.size(), p.second);
                if (number_to_canonical_name[p.second] == p.first) {
                    number_to_index.emplace(p.second, i);
                }
                ++i;
                data_index += p.first.size();
            }

            format(
                "};\n"
                "\n"
                "static const int $classname$_entries_by_number[] = {\n");
            for (const auto& p : number_to_index) {
                format("  $1$, // $2$ -> $3$\n", p.second, p.first,
                    number_to_canonical_name[p.first]);
            }
            format(
                "};\n"
                "\n");

            format(
                "const std::string& $classname$_Name(\n"
                "    $classname$ value) {\n"
                "  static const bool dummy =\n"
                "      ::$proto_ns$::internal::InitializeEnumStrings(\n"
                "          $classname$_entries,\n"
                "          $classname$_entries_by_number,\n"
                "          $1$, $classname$_strings);\n"
                "  (void) dummy;\n"
                "  int idx = ::$proto_ns$::internal::LookUpEnumName(\n"
                "      $classname$_entries,\n"
                "      $classname$_entries_by_number,\n"
                "      $1$, $classname$_to_underlying(value));\n"
                "  return idx == -1 ? ::$proto_ns$::internal::GetEmptyString() :\n"
                "                     $classname$_strings[idx].get();\n"
                "}\n",
                CountUniqueValues(descriptor_));
            format(
                "bool $classname$_Parse(\n"
                "    ::SHARK_NAMESPACE_ID::ConstStringParam name, $classname$* "
                "value) "
                "{\n"
                "  int int_value;\n"
                "  bool success = ::$proto_ns$::internal::LookUpEnumValue(\n"
                "      $classname$_entries, $1$, name, &int_value);\n"
                "  if (success) {\n"
                "    *value = static_cast<$classname$>(int_value);\n"
                "  }\n"
                "  return success;\n"
                "}\n",
                descriptor_->value_count());
        }
    }

} // namespace shark::compiler::cpp
