# Copyright (C) Kumo inc. and its affiliates.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

kmcmake_cc_interface(
        NAMESPACE ${PROJECT_NAME}
        NAME skills
        HEADERS
        skills.h
        version.h
        CXXOPTS
        ${KMCMAKE_CXX_OPTIONS}
        PUBLIC
)

kmcmake_cc_library(
        PUBLIC
        NAME shark-lite
        NAMESPACE ${PROJECT_NAME}
        SOURCES
        types/any_lite.cc
        arena.cc
        arenastring.cc
        arenaz_sampler.cc
        extension_set.cc
        generated/generated_enum_util.cc
        generated/generated_message_tctable_lite.cc
        generated/generated_message_util.cc
        generated/implicit_weak_message.cc
        generated/inlined_string_field.cc
        io/coded_stream.cc
        io/io_win32.cc
        io/strtod.cc
        io/zero_copy_stream.cc
        io/zero_copy_stream_impl.cc
        io/zero_copy_stream_impl_lite.cc
        types/map.cc
        message_lite.cc
        parse_context.cc
        repeated_field.cc
        repeated_ptr_field.cc
        stubs/bytestream.cc
        stubs/common.cc
        stubs/int128.cc
        stubs/status.cc
        stubs/statusor.cc
        stubs/stringpiece.cc
        stubs/stringprintf.cc
        stubs/structurally_valid.cc
        stubs/strutil.cc
        stubs/time.cc
        format/wired/wire_format_lite.cc
        CXXOPTS ${KMCMAKE_CXX_OPTIONS}
        LINKS ${KMCMAKE_DEPS_LINK}
)
kmcmake_cc_library(
        PUBLIC
        NAME shark
        NAMESPACE ${PROJECT_NAME}
        SOURCES
        types/any.cc
        types/any.skp.cc
        types/api.skp.cc
        compiler/importer.cc
        compiler/parser.cc
        meta.skp.cc
        descriptor.cc
        descriptor.skp.cc
        descriptor_database.cc
        types/duration.skp.cc
        dynamic_message.cc
        types/empty.skp.cc
        extension_set_heavy.cc
        types/field_mask.skp.cc
        generated/generated_message_bases.cc
        generated/generated_message_reflection.cc
        generated/generated_message_tctable_full.cc
        io/gzip_stream.cc
        io/printer.cc
        io/tokenizer.cc
        types/map_field.cc
        message.cc
        reflection_ops.cc
        service.cc
        types/source_context.skp.cc
        types/struct.skp.cc
        stubs/substitute.cc
        text_format.cc
        types/timestamp.skp.cc
        types/type.skp.cc
        unknown_field_set.cc
        util/delimited_message_util.cc
        util/field_comparator.cc
        util/field_mask_util.cc
        transfer/json/datapiece.cc
        transfer/json/default_value_objectwriter.cc
        transfer/json/error_listener.cc
        transfer/json/field_mask_utility.cc
        transfer/json/json_escaping.cc
        transfer/json/json_objectwriter.cc
        transfer/json/json_stream_parser.cc
        transfer/json/object_writer.cc
        transfer/json/proto_writer.cc
        transfer/json/protostream_objectsource.cc
        transfer/json/protostream_objectwriter.cc
        transfer/json/type_info.cc
        transfer/json/utility.cc
        transfer/json/json_util.cc
        util/message_differencer.cc
        util/time_util.cc
        transfer/json/type_resolver_util.cc
        format/wired/wire_format.cc
        types/wrappers.skp.cc
        CXXOPTS ${KMCMAKE_CXX_OPTIONS}
        LINKS ${PROJECT_NAME}::shark-lite_static ${KMCMAKE_DEPS_LINK}
)
kmcmake_cc_library(
        PUBLIC
        NAME sharkc-lib
        NAMESPACE ${PROJECT_NAME}
        SOURCES
        compiler/code_generator.cc
        compiler/command_line_interface.cc
        compiler/cpp/enum.cc
        compiler/cpp/enum_field.cc
        compiler/cpp/extension.cc
        compiler/cpp/field.cc
        compiler/cpp/file.cc
        compiler/cpp/generator.cc
        compiler/cpp/helpers.cc
        compiler/cpp/map_field.cc
        compiler/cpp/message.cc
        compiler/cpp/message_field.cc
        compiler/cpp/padding_optimizer.cc
        compiler/cpp/parse_function_generator.cc
        compiler/cpp/primitive_field.cc
        compiler/cpp/service.cc
        compiler/cpp/string_field.cc
        compiler/plugin.cc
        compiler/plugin.skp.cc
        compiler/python/generator.cc
        compiler/python/helpers.cc
        compiler/python/pyi_generator.cc
        compiler/subprocess.cc
        compiler/zip_writer.cc
        CXXOPTS ${KMCMAKE_CXX_OPTIONS}
        LINKS ${PROJECT_NAME}::shark_static
)
kmcmake_cc_binary(
        PUBLIC
        NAME sharkc
        SOURCES compiler/main.cc
        CXXOPTS ${KMCMAKE_CXX_OPTIONS}
        LINKS ${PROJECT_NAME}::sharkc-lib_static ${KMCMAKE_DEPS_LINK}
)
