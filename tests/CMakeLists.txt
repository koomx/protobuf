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

configure_file(${PROJECT_SOURCE_DIR}/tests/config.h.in ${PROJECT_SOURCE_DIR}/tests/config.h @ONLY)

find_package(GTest REQUIRED)

# Test protos are generated at build time by sharkc, next to the .proto.
# Library .pb.cc files under shark/ stay checked in and are not generated here.
file(GLOB_RECURSE TEST_PROTOS CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/*.proto)
set(lite_test_proto_files)
set(tests_proto_files)
foreach (proto_file ${TEST_PROTOS})
    string(REPLACE .proto .pb.cc pb_file ${proto_file})
    string(REPLACE .proto .pb.h pb_hdr ${proto_file})
    add_custom_command(
            OUTPUT ${pb_file} ${pb_hdr}
            DEPENDS sharkc ${proto_file}
            COMMAND sharkc ${proto_file}
            --proto_path=${PROJECT_SOURCE_DIR}
            --cpp_out=${PROJECT_SOURCE_DIR}
            --experimental_allow_proto3_optional
    )
    get_filename_component(_pb_name ${pb_file} NAME)
    if (_pb_name MATCHES "^(map_lite_unittest|unittest_import_lite|unittest_import_public_lite|unittest_lite)\\.pb\\.cc$")
        list(APPEND lite_test_proto_files ${pb_file})
    else ()
        list(APPEND tests_proto_files ${pb_file})
    endif ()
endforeach ()

kmcmake_cc_library(
        NAME shark-lite-test-common
        NAMESPACE ${PROJECT_NAME}
        SOURCES
        arena_test_util.cc
        map_lite_test_util.cc
        test_util_lite.cc
        ${lite_test_proto_files}
        CXXOPTS ${KMCMAKE_CXX_OPTIONS}
        LINKS ${PROJECT_NAME}::shark-lite_static GTest::gmock
)

kmcmake_cc_library(
        NAME shark-test-common
        NAMESPACE ${PROJECT_NAME}
        SOURCES
        arena_test_util.cc
        map_lite_test_util.cc
        test_util_lite.cc
        test_util.cc
        reflection_tester.cc
        testing/file.cc
        testing/googletest.cc
        ../shark/compiler/mock_code_generator.cc
        ${tests_proto_files}
        ${lite_test_proto_files}
        CXXOPTS ${KMCMAKE_CXX_OPTIONS}
        LINKS ${PROJECT_NAME}::shark_static GTest::gmock
)

kmcmake_cc_test(
        NAME suite
        MODULE ${PROJECT_NAME}
        SOURCES
        any_test.cc
        arena_unittest.cc
        arenastring_unittest.cc
        arenaz_sampler_test.cc
        compiler/annotation_test_util.cc
        compiler/command_line_interface_unittest.cc
        compiler/cpp/bootstrap_unittest.cc
        compiler/cpp/message_size_unittest.cc
        compiler/cpp/metadata_test.cc
        compiler/cpp/move_unittest.cc
        compiler/cpp/plugin_unittest.cc
        compiler/cpp/unittest.cc
        compiler/importer_unittest.cc
        compiler/parser_unittest.cc
        compiler/python/plugin_unittest.cc
        descriptor_database_unittest.cc
        descriptor_unittest.cc
        drop_unknown_fields_test.cc
        dynamic_message_unittest.cc
        extension_set_unittest.cc
        generated_message_reflection_unittest.cc
        generated_message_tctable_lite_test.cc
        inlined_string_field_unittest.cc
        io/coded_stream_unittest.cc
        io/io_win32_unittest.cc
        io/printer_unittest.cc
        io/tokenizer_unittest.cc
        io/zero_copy_stream_unittest.cc
        map_field_test.cc
        map_test.cc
        message_unittest.cc
        no_field_presence_test.cc
        preserve_unknown_enum_test.cc
        proto3_arena_lite_unittest.cc
        proto3_arena_unittest.cc
        proto3_lite_unittest.cc
        reflection_ops_unittest.cc
        repeated_field_reflection_unittest.cc
        repeated_field_unittest.cc
        stubs/bytestream_unittest.cc
        stubs/common_unittest.cc
        stubs/int128_unittest.cc
        stubs/status_test.cc
        stubs/statusor_test.cc
        stubs/stringpiece_unittest.cc
        stubs/stringprintf_unittest.cc
        stubs/structurally_valid_unittest.cc
        stubs/strutil_unittest.cc
        stubs/template_util_unittest.cc
        stubs/time_test.cc
        text_format_unittest.cc
        unknown_field_set_unittest.cc
        util/delimited_message_util_test.cc
        util/field_comparator_test.cc
        util/field_mask_util_test.cc
        util/internal/default_value_objectwriter_test.cc
        util/internal/json_objectwriter_test.cc
        util/internal/json_stream_parser_test.cc
        util/internal/protostream_objectsource_test.cc
        util/internal/protostream_objectwriter_test.cc
        util/internal/type_info_test_helper.cc
        util/json_util_test.cc
        util/message_differencer_unittest.cc
        util/time_util_test.cc
        util/type_resolver_util_test.cc
        well_known_types_unittest.cc
        wire_format_unittest.cc
        CXXOPTS ${KMCMAKE_CXX_OPTIONS}
        LINKS
        ${PROJECT_NAME}::shark-test-common_static
        ${PROJECT_NAME}::sharkc-lib_static
        ${PROJECT_NAME}::shark_static
        GTest::gmock_main
)

kmcmake_cc_test(
        NAME lite
        MODULE ${PROJECT_NAME}
        SOURCES lite_unittest.cc
        CXXOPTS ${KMCMAKE_CXX_OPTIONS}
        LINKS
        ${PROJECT_NAME}::shark-lite-test-common_static
        ${PROJECT_NAME}::shark-lite_static
        GTest::gmock_main
)

kmcmake_cc_test(
        NAME lite-arena
        MODULE ${PROJECT_NAME}
        SOURCES lite_arena_unittest.cc
        CXXOPTS ${KMCMAKE_CXX_OPTIONS}
        LINKS
        ${PROJECT_NAME}::shark-lite-test-common_static
        ${PROJECT_NAME}::shark-lite_static
        GTest::gmock_main
)

kmcmake_cc_binary(
        NAME test_plugin
        SOURCES
        compiler/test_plugin.cc
        testing/file.cc
        ../shark/compiler/mock_code_generator.cc
        CXXOPTS ${KMCMAKE_CXX_OPTIONS}
        LINKS
        ${PROJECT_NAME}::sharkc-lib_static
        ${PROJECT_NAME}::shark_static
        GTest::gmock
)
