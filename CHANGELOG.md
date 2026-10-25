# Changelog

新任务加在最上面，一节一件事。步骤做完后，把状态改成完成，并写上实际结果。未做的步骤保持未做。

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
