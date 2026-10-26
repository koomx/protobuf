# Changelog

新任务加在最上面，一节一件事。步骤做完后，把状态改成完成，并写上实际结果。未做的步骤保持未做。

## 补上 map value 的窄整数序列化

日期：2026-10-26

`SerializeMapValueRefWithCachedSizes` 缺 `INT8`、`UINT8`、`INT16`、`UINT16`。反射序列化 map 时这四种 value 写不出 value 字节。生成代码的 `SerializeToString` 不走这个函数，这一节不改生成器。

| 步骤 | 内容 | 状态 | 完成效果 |
| --- | --- | --- | --- |
| 1 | 在 `shark/format/wired/wire_format.cc` 的 `SerializeMapValueRefWithCachedSizes` 里，按上面 `SerializeMapKeyWithCachedSizes` 的对应关系加四个 `CASE_TYPE`：`INT8` 和 `INT16` 用 `Int32, Int32`，`UINT8` 和 `UINT16` 用 `UInt32, UInt32`。宏里的字段号保持 2。不在这里做范围检查。同一文件里 key 序列化、key/value 字节数、普通字段反射序列化已经有这四个 case，不动 | 完成 | 四个 `CASE_TYPE` 已加在 `INT32` 前面。字段号仍是 2。写成 32 位 varint，没有范围检查 |
| 2 | `tests/types/narrow_int.shark` 的 `NarrowMaps` 现在 value 都是 `int32`，补四个 value 为窄整数的字段：`map<int32, int8> vi8 = 5`、`map<int32, uint8> vu8 = 6`、`map<int32, int16> vi16 = 7`、`map<int32, uint16> vu16 = 8`。在 `tests/types/narrow_int_reflect_test.cc` 加一个测试：每个 map 写一条，value 分别是 `-128`、`255`、`-32768`、`65535`。生成代码 `SerializeToString` 的字节，和同一条消息走 `WireFormat::SerializeWithCachedSizes` 的字节必须相同。再用 `DynamicMessage` 把生成字节解析后 `SerializeToString`，结果仍和生成字节相同。比较的是字节，不只是长度 | 完成 | 四个 map 已加。`NarrowIntReflectTest.MapValueSerializeMatchesGenerated` 通过：反射序列化和 `DynamicMessage` 的字节都与生成代码相同 |

## 字段类型 10 定义为 LIST

日期：2026-10-26

排在「去掉 group」之后。编号 10 现在空着。这一节还不能写执行步骤：下面几项没有定，定之前不动代码。

未定：

- `list<T>` 解析之后，字段类型是否写成 `LIST`。`label` 是否还保留 `LABEL_REPEATED`。
- `map` 现在也是 `is_repeated()` 为真，类型仍是 message。改成看 `LIST` 之后，map 用什么判断。
- `LIST` 的 wire type 是否仍是元素自己的 wire type，还是另有编码。
- `is_repeated()` 是删除，还是改成 `type() == LIST` 的别名。

## 去掉 SYNTAX_PROTO3

日期：2026-10-26

`SYNTAX_PROTO3` 从枚举里删除，编号不保留。旧数据里的原编号按不认识的枚举值处理。排在「去掉 group」之后做。

| 步骤 | 内容 | 状态 | 完成效果 |
| --- | --- | --- | --- |
| 1 | 从 `FileDescriptor::Syntax` 去掉 `SYNTAX_PROTO3`。`descriptor.cc`、`command_line_interface.cc`、Python 生成器里对这个值的分支删掉。`FileDescriptorProto.syntax` 仍是字符串，内容是 `"proto3"` 时继续由现有解析失败，不再靠这个枚举值 | 未做 | |
| 2 | 从 `shark/types/type.shark` 的 `Syntax` 枚举去掉 `SYNTAX_PROTO3`，并重新生成 `type.skp`。`ides/vscode` 和 `ides/clion` 里的同名副本一起改。`proto_writer.cc` 里和 `SYNTAX_PROTO3` 比较的分支删掉 | 未做 | |
| 3 | `descriptor_unittest` 里期望编号为 `3` 的断言删掉。其余还引用 `SYNTAX_PROTO3` 的测试改为不认识的枚举值 | 未做 | |

## 去掉 group

日期：2026-10-26

schema 不再接受 group 字段。`optional group`、`required group`、`repeated group` 都不能再声明。`TYPE_GROUP` 和 `FieldType::GROUP` 删掉。编号 10 留空，不分配给其他类型，这一节不定义 `LIST`。旧描述符里的 10 按不认识的枚举值处理。这一节不改 `list` 语法，也不改 `repeated`。

| 步骤 | 内容 | 状态 | 完成效果 |
| --- | --- | --- | --- |
| 1 | `shark/compiler/parser.cc` 的类型表去掉 `"group"`。`group` 不单独报错，和其它不在类型表里的名字一样走 `ParseUserDefinedType`。不读 `{ ... }`，不生成嵌套消息，不把字段名改成小写。删掉 `Group names must start with a capital letter` 和 `Missing group body`。`ParseType` 的 `TYPE_GROUP` 分支删掉 | 完成 | 类型表没有 `"group"`。`optional group bar = 1;` 和 `optional xxx bar = 1;` 一样，建描述符时再报 `"group" is not defined.` |
| 2 | `shark/descriptor.shark` 的 `FieldDescriptorProto.Type`、`shark/descriptor.h` 的 `FieldType`、`shark/format/wired/wire_format_lite.h` 的 `WireFormatLiteFieldType`、`shark/types/type.shark` 的 `Field.Kind` 去掉 group。编号 10 不写新名字。`ides/vscode` 和 `ides/clion` 里这两份 schema 的副本一起改。用 `sharkc` 重新生成 `descriptor.skp` 和 `type.skp`，不手改生成文件里的描述符字节 | 完成 | 四处枚举都没有 group。编号 10 空着。IDE 副本已改。`descriptor.skp`、`type.skp` 由 `sharkc` 重新生成 |
| 3 | `descriptor.cc` 的 `kTypeToName`、`kFieldTypeToCppTypeMap` 在下标 10 不再写 `"group"` 或 message。`DebugString` 不再把 group 的嵌套类型从消息体里拿掉，也不再把字段打成 `group Name = N { ... }`。建描述符时不为类型值 10 写单独错误 | 完成 | 下标 10 的名字是 `"ERROR"`，cpp 类型是 0。没有单独的类型值 10 错误 |
| 4 | 下面 `.shark` 里的 `optional group`、`required group`、`repeated group` 整段删掉，包括声明体。其余字段不动：`tests/unittest.shark`、`tests/unittest_lite.shark`、`tests/unittest_custom_options.shark`、`tests/unittest_mset.shark`、`tests/test_messages_proto2.shark`、`tests/util/message_differencer_unittest.shark`、`tests/transfer/json/json_format.shark`、`tests/transfer/json/testdata/books.shark`、`benchmark/google_size.shark`、`benchmark/datasets/google_message2/benchmark_message2.shark`、`benchmark/datasets/google_message3/benchmark_message3.shark`、`benchmark_message3_1.shark` 到 `benchmark_message3_6.shark`、`benchmark/datasets/google_message4/benchmark_message4.shark`、`benchmark_message4_1.shark` | 完成 | 这些文件里的 group 声明已删。`oneof` 里的 `group` 同样删掉，否则 `sharkc` 编不过。`tests` 和 `benchmark` 下的 `.shark` 已用当前 `sharkc` 编过 |
| 5 | 只覆盖 group 的测试删掉：`parser_unittest.cc` 的 `ParseMessageTest.Group`、`ParseErrorTest.DefaultValueForGroup`、`GroupNotCapitalized`、`GroupMissingBody`、`SourceInfoTest.Groups`；`unknown_field_set_unittest.cc` 的 `UnknownFieldSetTest.Group`；text format 数据里的 `OptionalGroup`、`RepeatedGroup` 段。`message_unittest.inc`、`wire_format_unittest.inc`、`generated_message_reflection_unittest.cc`、`dynamic_message_unittest.cc`、`extension_set_unittest.cc`、`reflection_ops_unittest.cc`、`message_differencer_unittest.cc`、`text_format_unittest.cc`、`descriptor_unittest.cc` 里还测别的行为的，只去掉 group 字段和对应断言 | 完成 | 列出的 group 测试已删。MessageSet 只按 type id 和 length-delimited message 配对，不再用 start/end group。`ctest` 6 项全部通过 |
| 6 | 生成器去掉 group 分支：`shark/compiler/cpp/parse_function_generator.cc`、`helpers.cc`、`field.cc`、`primitive_field.cc`、`command_line_interface.cc`。不再生成 `ParseGroup`、start-group、end-group 的读写 | 完成 | 这些生成器文件不再按 group 生成读写。`ninja sharkc` 通过 |
| 7 | 删掉 `WIRETYPE_START_GROUP`、`WIRETYPE_END_GROUP`。删掉 `SkipField`、`FieldParser`、`_InternalParse` 里 wire type 3 和 4 的 case，使这两个值进入这些函数现有的 default。删掉 `MessageSet` 用这两种 tag 做分组的读写。删掉 `UnknownField::TYPE_GROUP`，以及 `unknown_field_set.cc`、`wire_format.cc`、`message_differencer.cc`、`text_format.cc` 里读写它的代码 | 完成 | 这两个名字已删，wire type 3 和 4 走原有 default。`UnknownField` 不再有 `TYPE_GROUP`、`AddGroup`、`group()`。`tests/shark_suite` 能链接 |

## 编辑器去掉 proto3

日期：2026-10-26

| 步骤 | 内容 | 状态 | 完成效果 |
| --- | --- | --- | --- |
| 1 | CLion 不再按 proto3 规则检查字段，`syntax = "proto3"` 标成错误。补全不再给出 proto3 字段标签。VS Code 插入语法的片段改为 proto2 | 完成 | `Proto3Annotator` 只在 syntax 语句上报 `syntax "proto3" is not supported`。补全去掉 `createProto3FieldLabels`。片段 `sp2` 插入 `syntax = "proto2";`。IDE 测试没有跑 |

## Any 的设置状态

日期：2026-10-26

`Any` 改成 proto2 之后，外层字段和两个成员各自有设置位。序列化不校验 `type_url` 和 `value` 是否为空、是否匹配。

| 步骤 | 内容 | 状态 | 完成效果 |
| --- | --- | --- | --- |
| 1 | `PackFrom` 成功后置上 `type_url` 和 `value` 两个成员的设置位。生成器里的 `PackFrom` 写出同样的设置。只 `mutable` 外层或成员、没有写入时，不额外置位 | 完成 | `any.skp.h` 的四个 `PackFrom` 在打包成功后执行 `_impl_._has_bits_[0] \|= 0x00000003u`。失败不置位。`shark/compiler/cpp/message.cc` 对 descriptor 和 lite 的 `PackFrom` 都生成这一行。`any.cc` 和 `any_lite.cc` 仍只写字符串 |
| 2 | JSON 测试按 proto2 的空字符串处理 | 完成 | `JsonUtilTest.ParsePrimitiveMapIn` 解析后期望 `other` 已设置且值为空，调试输出为 `other: ""`。`shark_json` 里 Any 相关 14 个测试通过，原先失败的 7 个包含在内 |

## 移除 proto3

日期：2026-10-26

proto2 继续作为唯一语法。设置状态留在字段上，并在以后的序列化里使用。这一节不改 tag，也不改 `int8`、`int16`、`uint8`、`uint16` 的 payload。

| 步骤 | 内容 | 状态 | 完成效果 |
| --- | --- | --- | --- |
| 1 | 把 `shark/types` 下 `any`、`duration`、`timestamp`、`empty`、`struct`、`wrappers`、`field_mask`、`source_context`、`type`、`api` 改成 `syntax = "proto2"`。单值写 `optional`，重复字段保持 `repeated`。只重新生成这些文件的 `.skp` | 完成 | 这 10 个文件已是 proto2。单值有 `optional`，`repeated` 和 `map` 未改标签，`Value` 的 oneof 成员保持无标签。对应 `.skp` 已重新生成，单值带有 `has`。`Syntax` 枚举里的 `SYNTAX_PROTO3` 仍保留 |
| 2 | 解析器只接受 `syntax = "proto2"`。`proto3`、缺少 syntax、以及 proto3 的隐式 `optional` 都报错。拒绝 `proto3_optional`，不再生成只含一个字段的 oneof | 完成 | `Parser` 遇到非 `proto2` 或文件开头没有 syntax 时直接报错，不再默认成 proto2。不再给 proto3 的 `optional` 写 `proto3_optional`，也不再生成单字段 oneof。解析器单测、两条 proto3 optional 的命令行测试，以及 Python import 测试已按这个结果改过。内存里的 `syntax = "proto3"` 描述符在第 4 步拒绝 |
| 3 | 去掉只因 proto3 才存在的行为。单值是否设置只看 `optional` 和 `required`。`repeated` 仅在写了 `[packed = true]` 时打包。enum 不再默认接受未知值。去掉 proto3 对 extension、group、`required` 的禁止 | 完成 | `is_packed()` 只在字段写了 `packed` 时为真。未知枚举值进 unknown fields，不再按 proto3 留下。`ValidateProto3` 不再调用，所以 `required`、group、extension、默认值、enum 首值、JSON 重名这些 proto3 限制都不再生效。描述符单测里对应用例改为能够建文件。编译器的 `IsProto3` 已在第 6 步删除 |
| 4 | `FileDescriptor::SYNTAX_PROTO3` 的编号保留并拒绝，不把该编号改作它用。`proto3_optional` 的字段号保留，读到就报错 | 完成 | `SYNTAX_PROTO3` 仍是 3。描述符里 `syntax` 为 `proto3` 时建文件失败，错误是 `syntax "proto3" is not supported`。`FieldDescriptorProto.proto3_optional` 仍是字段 17，置上就报 `proto3_optional is not supported`。`descriptor_unittest` 里原来用 proto3 建文件的用例改成 proto2，并加上这两条拒绝测试 |
| 5 | 删除只覆盖 proto3 语义的测试。测 Any、map、内置类型、未知字段和 JSON 的 schema 改成 proto2，不删这些测试 | 完成 | 已删除 `unittest_proto3*`、`proto3_*unittest`、`unittest_no_field_presence`、`preserve_unknown_enum*`、`test_messages_proto3`、`golden_message_proto3`、`testdata/proto3.shark`。`dynamic_message_unittest` 里的 proto3 用例已去掉。Any、map、well-known、drop-unknown 和 JSON schema 已改为 proto2 并重新生成 `.skp`。JSON 里未知枚举那几条 proto3 用例已去掉。解析器拒绝 proto3 已在第 2 步完成 |
| 6 | 删除编译器中的 `IsProto3`、默认打包，以及不为单值生成 `has` 的分支。Python 生成器只在枚举开关无法编译时改对应分支 | 完成 | `IsProto3` 已删除。单值都生成 `has`。未知枚举不再按 proto3 留在字段里。UTF-8 不再因为 proto3 走严格检查。Python 的 `SYNTAX_PROTO3` 分支还能编译，未改 |
| 7 | `ides/vscode` 和 `ides/clion` 的 proto3 编辑器支持另作一节，不放进这一节 | 完成 | 见最上面一节「编辑器去掉 proto3」 |
