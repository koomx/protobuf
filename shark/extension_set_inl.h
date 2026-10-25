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

#pragma once

#include <shark/extension_set.h>
#include <shark/metadata_lite.h>
#include <shark/parse_context.h>

namespace shark::internal {

template <typename T>
const char* ExtensionSet::ParseFieldWithExtensionInfo(
    int number, bool was_packed_on_wire, const ExtensionInfo& extension,
    InternalMetadata* metadata, const char* ptr, internal::ParseContext* ctx) {
  if (was_packed_on_wire) {
    switch (static_cast<WireFormatLiteFieldType>(extension.type)) {
#define HANDLE_TYPE(UPPERCASE, CPP_CAMELCASE)                                \
  case WireFormatLiteFieldType::TYPE_##UPPERCASE:                                     \
    return internal::Packed##CPP_CAMELCASE##Parser(                          \
        MutableRawRepeatedField(number, extension.type, extension.is_packed, \
                                extension.descriptor),                       \
        ptr, ctx);
      HANDLE_TYPE(INT32, Int32);
      HANDLE_TYPE(INT64, Int64);
      HANDLE_TYPE(UINT32, UInt32);
      HANDLE_TYPE(UINT64, UInt64);
      HANDLE_TYPE(SINT32, SInt32);
      HANDLE_TYPE(SINT64, SInt64);
      HANDLE_TYPE(FIXED32, Fixed32);
      HANDLE_TYPE(FIXED64, Fixed64);
      HANDLE_TYPE(SFIXED32, SFixed32);
      HANDLE_TYPE(SFIXED64, SFixed64);
      HANDLE_TYPE(FLOAT, Float);
      HANDLE_TYPE(DOUBLE, Double);
      HANDLE_TYPE(BOOL, Bool);
#undef HANDLE_TYPE

      case WireFormatLiteFieldType::TYPE_ENUM:
        return internal::PackedEnumParserArg<T>(
            MutableRawRepeatedField(number, extension.type, extension.is_packed,
                                    extension.descriptor),
            ptr, ctx, extension.enum_validity_check.func,
            extension.enum_validity_check.arg, metadata, number);
      case WireFormatLiteFieldType::TYPE_STRING:
      case WireFormatLiteFieldType::TYPE_BYTES:
      case WireFormatLiteFieldType::TYPE_GROUP:
      case WireFormatLiteFieldType::TYPE_MESSAGE:
        GOOGLE_LOG(FATAL) << "Non-primitive types can't be packed.";
        break;
    }
  } else {
    switch (static_cast<WireFormatLiteFieldType>(extension.type)) {
#define HANDLE_VARINT_TYPE(UPPERCASE, CPP_CAMELCASE)                        \
  case WireFormatLiteFieldType::TYPE_##UPPERCASE: {                                  \
    uint64_t value;                                                         \
    ptr = VarintParse(ptr, &value);                                         \
    SHARK_PARSER_ASSERT(ptr);                                    \
    if (extension.is_repeated) {                                            \
      Add##CPP_CAMELCASE(number, static_cast<FieldTypeUint8>(WireFormatLiteFieldType::TYPE_##UPPERCASE),          \
                         extension.is_packed, value, extension.descriptor); \
    } else {                                                                \
      Set##CPP_CAMELCASE(number, static_cast<FieldTypeUint8>(WireFormatLiteFieldType::TYPE_##UPPERCASE), value,   \
                         extension.descriptor);                             \
    }                                                                       \
  } break

      HANDLE_VARINT_TYPE(INT32, Int32);
      HANDLE_VARINT_TYPE(INT64, Int64);
      HANDLE_VARINT_TYPE(UINT32, UInt32);
      HANDLE_VARINT_TYPE(UINT64, UInt64);
      HANDLE_VARINT_TYPE(BOOL, Bool);
#undef HANDLE_VARINT_TYPE
#define HANDLE_SVARINT_TYPE(UPPERCASE, CPP_CAMELCASE, SIZE)                 \
  case WireFormatLiteFieldType::TYPE_##UPPERCASE: {                                  \
    uint64_t val;                                                           \
    ptr = VarintParse(ptr, &val);                                           \
    SHARK_PARSER_ASSERT(ptr);                                    \
    auto value = WireFormatLite::ZigZagDecode##SIZE(val);                   \
    if (extension.is_repeated) {                                            \
      Add##CPP_CAMELCASE(number, static_cast<FieldTypeUint8>(WireFormatLiteFieldType::TYPE_##UPPERCASE),          \
                         extension.is_packed, value, extension.descriptor); \
    } else {                                                                \
      Set##CPP_CAMELCASE(number, static_cast<FieldTypeUint8>(WireFormatLiteFieldType::TYPE_##UPPERCASE), value,   \
                         extension.descriptor);                             \
    }                                                                       \
  } break

      HANDLE_SVARINT_TYPE(SINT32, Int32, 32);
      HANDLE_SVARINT_TYPE(SINT64, Int64, 64);
#undef HANDLE_SVARINT_TYPE
#define HANDLE_FIXED_TYPE(UPPERCASE, CPP_CAMELCASE, CPPTYPE)                \
  case WireFormatLiteFieldType::TYPE_##UPPERCASE: {                                  \
    auto value = UnalignedLoad<CPPTYPE>(ptr);                               \
    ptr += sizeof(CPPTYPE);                                                 \
    if (extension.is_repeated) {                                            \
      Add##CPP_CAMELCASE(number, static_cast<FieldTypeUint8>(WireFormatLiteFieldType::TYPE_##UPPERCASE),          \
                         extension.is_packed, value, extension.descriptor); \
    } else {                                                                \
      Set##CPP_CAMELCASE(number, static_cast<FieldTypeUint8>(WireFormatLiteFieldType::TYPE_##UPPERCASE), value,   \
                         extension.descriptor);                             \
    }                                                                       \
  } break

      HANDLE_FIXED_TYPE(FIXED32, UInt32, uint32_t);
      HANDLE_FIXED_TYPE(FIXED64, UInt64, uint64_t);
      HANDLE_FIXED_TYPE(SFIXED32, Int32, int32_t);
      HANDLE_FIXED_TYPE(SFIXED64, Int64, int64_t);
      HANDLE_FIXED_TYPE(FLOAT, Float, float);
      HANDLE_FIXED_TYPE(DOUBLE, Double, double);
#undef HANDLE_FIXED_TYPE

      case WireFormatLiteFieldType::TYPE_ENUM: {
        uint64_t val;
        ptr = VarintParse(ptr, &val);
        SHARK_PARSER_ASSERT(ptr);
        int value = val;

        if (!extension.enum_validity_check.func(
                extension.enum_validity_check.arg, value)) {
          WriteVarint(number, val, metadata->mutable_unknown_fields<T>());
        } else if (extension.is_repeated) {
          AddEnum(number, static_cast<FieldTypeUint8>(WireFormatLiteFieldType::TYPE_ENUM), extension.is_packed, value,
                  extension.descriptor);
        } else {
          SetEnum(number, static_cast<FieldTypeUint8>(WireFormatLiteFieldType::TYPE_ENUM), value,
                  extension.descriptor);
        }
        break;
      }

      case WireFormatLiteFieldType::TYPE_BYTES:
      case WireFormatLiteFieldType::TYPE_STRING: {
        std::string* value =
            extension.is_repeated
                ? AddString(number, static_cast<FieldTypeUint8>(WireFormatLiteFieldType::TYPE_STRING),
                            extension.descriptor)
                : MutableString(number, static_cast<FieldTypeUint8>(WireFormatLiteFieldType::TYPE_STRING),
                                extension.descriptor);
        int size = ReadSize(&ptr);
        SHARK_PARSER_ASSERT(ptr);
        return ctx->ReadString(ptr, size, value);
      }

      case WireFormatLiteFieldType::TYPE_GROUP: {
        MessageLite* value =
            extension.is_repeated
                ? AddMessage(number, static_cast<FieldTypeUint8>(WireFormatLiteFieldType::TYPE_GROUP),
                             *extension.message_info.sharktype,
                             extension.descriptor)
                : MutableMessage(number, static_cast<FieldTypeUint8>(WireFormatLiteFieldType::TYPE_GROUP),
                                 *extension.message_info.sharktype,
                                 extension.descriptor);
        uint32_t tag = (number << 3) + WireFormatLite::WIRETYPE_START_GROUP;
        return ctx->ParseGroup(value, ptr, tag);
      }

      case WireFormatLiteFieldType::TYPE_MESSAGE: {
        MessageLite* value =
            extension.is_repeated
                ? AddMessage(number, static_cast<FieldTypeUint8>(WireFormatLiteFieldType::TYPE_MESSAGE),
                             *extension.message_info.sharktype,
                             extension.descriptor)
                : MutableMessage(number, static_cast<FieldTypeUint8>(WireFormatLiteFieldType::TYPE_MESSAGE),
                                 *extension.message_info.sharktype,
                                 extension.descriptor);
        return ctx->ParseMessage(value, ptr);
      }
    }
  }
  return ptr;
}

template <typename Msg, typename T>
const char* ExtensionSet::ParseMessageSetItemTmpl(
    const char* ptr, const Msg* extendee, internal::InternalMetadata* metadata,
    internal::ParseContext* ctx) {
  std::string payload;
  uint32_t type_id;
  enum class State { kNoTag, kHasType, kHasPayload, kDone };
  State state = State::kNoTag;

  while (!ctx->Done(&ptr)) {
    uint32_t tag = static_cast<uint8_t>(*ptr++);
    if (tag == WireFormatLite::kMessageSetTypeIdTag) {
      uint64_t tmp;
      ptr = ParseBigVarint(ptr, &tmp);
      SHARK_PARSER_ASSERT(ptr);
      if (state == State::kNoTag) {
        type_id = tmp;
        state = State::kHasType;
      } else if (state == State::kHasPayload) {
        type_id = tmp;
        ExtensionInfo extension;
        bool was_packed_on_wire;
        if (!FindExtension(2, type_id, extendee, ctx, &extension,
                           &was_packed_on_wire)) {
          WriteLengthDelimited(type_id, payload,
                               metadata->mutable_unknown_fields<T>());
        } else {
          MessageLite* value =
              extension.is_repeated
                  ? AddMessage(type_id, static_cast<FieldTypeUint8>(WireFormatLiteFieldType::TYPE_MESSAGE),
                               *extension.message_info.sharktype,
                               extension.descriptor)
                  : MutableMessage(type_id, static_cast<FieldTypeUint8>(WireFormatLiteFieldType::TYPE_MESSAGE),
                                   *extension.message_info.sharktype,
                                   extension.descriptor);

          const char* p;
          // We can't use regular parse from string as we have to track
          // proper recursion depth and descriptor pools.
          ParseContext tmp_ctx(ctx->depth(), false, &p, payload);
          tmp_ctx.data().pool = ctx->data().pool;
          tmp_ctx.data().factory = ctx->data().factory;
          SHARK_PARSER_ASSERT(value->_InternalParse(p, &tmp_ctx) &&
                                         tmp_ctx.EndedAtLimit());
        }
        state = State::kDone;
      }
    } else if (tag == WireFormatLite::kMessageSetMessageTag) {
      if (state == State::kHasType) {
        ptr = ParseFieldMaybeLazily(static_cast<uint64_t>(type_id) * 8 + 2, ptr,
                                    extendee, metadata, ctx);
        SHARK_PARSER_ASSERT(ptr != nullptr);
        state = State::kDone;
      } else {
        std::string tmp;
        int32_t size = ReadSize(&ptr);
        SHARK_PARSER_ASSERT(ptr);
        ptr = ctx->ReadString(ptr, size, &tmp);
        SHARK_PARSER_ASSERT(ptr);
        if (state == State::kNoTag) {
          payload = std::move(tmp);
          state = State::kHasPayload;
        }
      }
    } else {
      ptr = ReadTag(ptr - 1, &tag);
      if (tag == 0 || (tag & 7) == 4) {
        ctx->SetLastTag(tag);
        return ptr;
      }
      ptr = ParseField(tag, ptr, extendee, metadata, ctx);
      SHARK_PARSER_ASSERT(ptr);
    }
  }
  return ptr;
}

}  // namespace shark::internal

