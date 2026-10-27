// Tag codec for the shark wire format.
//
// Single source of truth for the tag bit layout. All tag encoding/decoding
// (runtime, reflection, generated code) must go through these functions —
// never write raw shift/mask arithmetic at call sites.
//
// Layout (32-bit tag, varint-coded on the wire):
//
//   bit  31..........5  4      3..0
//        field number   is_set  wire type
//        27 bits        1 bit   4 bits
//
// is_set is reserved for future presence signalling; writers set it to 1
// unconditionally and readers ignore it.
//
// Wire type values remain 0,1,2 (data), 3,4 (legacy groups), 5. The 4-bit
// field allows future types; values 6..15 are currently unassigned and
// rejected on input.
//
// This header is a dependency leaf: it must only include <cstdint>.

#pragma once

#include <cstdint>

// The codec lives in shark::tag so that every shark sub-namespace (io,
// internal, transfer, ...) can refer to it unqualified as `tag::`.
// shark::internal::tag is an alias for generated/runtime code that fully
// qualifies via ::shark::internal::tag.
namespace shark::tag {

// --- Layout constants -------------------------------------------------

inline constexpr int kWireTypeBits = 4;
inline constexpr uint32_t kWireTypeMask = (1u << kWireTypeBits) - 1;  // 0xF

// Reserved presence bit. Writers set it; readers ignore it (for now).
inline constexpr int kSetFlagShift = kWireTypeBits;               // 4
inline constexpr uint32_t kSetFlagMask = 1u << kSetFlagShift;     // 0x10

inline constexpr int kFieldShift = kWireTypeBits + 1;             // 5
inline constexpr uint32_t kMaxFieldNumber = (1u << 27) - 1;

inline constexpr uint32_t kMaxWireType = 5;

inline constexpr uint32_t kWireTypeEndGroup = 4;

// Largest field number whose tag still fits in a two-byte varint
// (14 payload bits - 5 tag overhead bits = 9 field-number bits).
inline constexpr uint32_t kMaxNumberFor2ByteTag = (1u << (14 - kFieldShift)) - 1;

// --- Codec ------------------------------------------------------------

// Builds a tag. `wire_type` must be <= kMaxWireType; `is_set` defaults to
// true (writers currently set the presence bit unconditionally).
constexpr uint32_t Make(uint32_t field_number, uint32_t wire_type,
                        bool is_set = true) {
  return (field_number << kFieldShift) | (is_set ? kSetFlagMask : 0u) |
         (wire_type & kWireTypeMask);
}

constexpr uint32_t FieldNumber(uint32_t tag) { return tag >> kFieldShift; }

constexpr uint32_t WireType(uint32_t tag) { return tag & kWireTypeMask; }

constexpr bool IsSet(uint32_t tag) { return (tag & kSetFlagMask) != 0; }

// A tag is decodable iff its field number is in [1, kMaxFieldNumber] and its
// wire type is assigned. Parsers must reject (not skip) invalid tags.
// Field numbers above kMaxFieldNumber cannot occur in a 32-bit tag under the
// current layout; the explicit bound documents the contract and guards the
// check for wider inputs.
constexpr bool IsValid(uint32_t tag) {
  uint32_t number = FieldNumber(tag);
  return number != 0 && number <= kMaxFieldNumber &&
         WireType(tag) <= kMaxWireType;
}

}  // namespace shark::tag

namespace shark::internal {
namespace tag = ::shark::tag;
}  // namespace shark::internal
