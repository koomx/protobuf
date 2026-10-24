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
        any_lite.cc
        arena.cc
        arenastring.cc
        arenaz_sampler.cc
        extension_set.cc
        generated_enum_util.cc
        generated_message_tctable_lite.cc
        generated_message_util.cc
        implicit_weak_message.cc
        inlined_string_field.cc
        io/coded_stream.cc
        io/io_win32.cc
        io/strtod.cc
        io/zero_copy_stream.cc
        io/zero_copy_stream_impl.cc
        io/zero_copy_stream_impl_lite.cc
        map.cc
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
        wire_format_lite.cc
        CXXOPTS ${KMCMAKE_CXX_OPTIONS}
        LINKS ${KMCMAKE_DEPS_LINK}
)
kmcmake_cc_library(
        PUBLIC
        NAME shark
        NAMESPACE ${PROJECT_NAME}
        SOURCES
        any.cc
        any.pb.cc
        api.pb.cc
        compiler/importer.cc
        compiler/parser.cc
        descriptor.cc
        descriptor.pb.cc
        descriptor_database.cc
        duration.pb.cc
        dynamic_message.cc
        empty.pb.cc
        extension_set_heavy.cc
        field_mask.pb.cc
        generated_message_bases.cc
        generated_message_reflection.cc
        generated_message_tctable_full.cc
        io/gzip_stream.cc
        io/printer.cc
        io/tokenizer.cc
        map_field.cc
        message.cc
        reflection_ops.cc
        service.cc
        source_context.pb.cc
        struct.pb.cc
        stubs/substitute.cc
        text_format.cc
        timestamp.pb.cc
        type.pb.cc
        unknown_field_set.cc
        util/delimited_message_util.cc
        util/field_comparator.cc
        util/field_mask_util.cc
        util/internal/datapiece.cc
        util/internal/default_value_objectwriter.cc
        util/internal/error_listener.cc
        util/internal/field_mask_utility.cc
        util/internal/json_escaping.cc
        util/internal/json_objectwriter.cc
        util/internal/json_stream_parser.cc
        util/internal/object_writer.cc
        util/internal/proto_writer.cc
        util/internal/protostream_objectsource.cc
        util/internal/protostream_objectwriter.cc
        util/internal/type_info.cc
        util/internal/utility.cc
        util/json_util.cc
        util/message_differencer.cc
        util/time_util.cc
        util/type_resolver_util.cc
        wire_format.cc
        wrappers.pb.cc
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
        compiler/plugin.pb.cc
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
