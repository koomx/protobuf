# Changelog

新任务加在最上面，一节一件事。步骤做完后，把状态改成完成，并写上实际结果。未做的步骤保持未做。

## 移除 proto3

日期：2026-10-26

proto2 继续作为唯一语法。设置状态留在字段上，并在以后的序列化里使用。这一节不改 tag，也不改 `int8`、`int16`、`uint8`、`uint16` 的 payload。

| 步骤 | 内容 | 状态 | 完成效果 |
| --- | --- | --- | --- |
| 1 | 把 `shark/types` 下 `any`、`duration`、`timestamp`、`empty`、`struct`、`wrappers`、`field_mask`、`source_context`、`type`、`api` 改成 `syntax = "proto2"`。单值写 `optional`，重复字段保持 `repeated`。只重新生成这些文件的 `.skp` | 完成 | 这 10 个文件已是 proto2。单值有 `optional`，`repeated` 和 `map` 未改标签，`Value` 的 oneof 成员保持无标签。对应 `.skp` 已重新生成，单值带有 `has`。`Syntax` 枚举里的 `SYNTAX_PROTO3` 仍保留 |
| 2 | 解析器只接受 `syntax = "proto2"`。`proto3`、缺少 syntax、以及 proto3 的隐式 `optional` 都报错。拒绝 `proto3_optional`，不再生成只含一个字段的 oneof | 未做 | |
| 3 | 去掉只因 proto3 才存在的行为。单值是否设置只看 `optional` 和 `required`。`repeated` 仅在写了 `[packed = true]` 时打包。enum 不再默认接受未知值。去掉 proto3 对 extension、group、`required` 的禁止 | 未做 | |
| 4 | `FileDescriptor::SYNTAX_PROTO3` 的编号保留并拒绝，不把该编号改作它用。`proto3_optional` 的字段号保留，读到就报错 | 未做 | |
| 5 | 删除只覆盖 proto3 语义的测试。测 Any、map、内置类型、未知字段和 JSON 的 schema 改成 proto2，不删这些测试 | 完成 | 已删除 `unittest_proto3*`、`proto3_*unittest`、`unittest_no_field_presence`、`preserve_unknown_enum*`、`test_messages_proto3`、`golden_message_proto3`、`testdata/proto3.shark`。`dynamic_message_unittest` 里的 proto3 用例已去掉。Any、map、well-known、drop-unknown 和 JSON schema 已改为 proto2 并重新生成 `.skp`。JSON 里未知枚举那几条 proto3 用例已去掉。解析器仍接受 proto3，这是第 2 步 |
| 6 | 删除编译器中的 `IsProto3`、默认打包，以及不为单值生成 `has` 的分支。Python 生成器只在枚举开关无法编译时改对应分支 | 未做 | |
| 7 | `ides/vscode` 和 `ides/clion` 的 proto3 编辑器支持另作一节，不放进这一节 | 未做 | |
