/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_ABI_HERMES_ABI_H
#define HERMES_ABI_HERMES_ABI_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

struct HermesABIRuntimeConfig;
struct HermesABIRuntime;
struct HermesABIManagedPointer;
struct HermesABIGrowableBuffer;
struct HermesABIBuffer;
struct HermesABIPropNameIDList;

/// Define the structure for references to pointer types in JS (e.g. string,
/// object, BigInt).
/// TODO: Replace jsi::PointerValue itself with this C implementation to
/// eliminate pointer management overhead in the JSI wrapper.
struct HermesABIManagedPointerVTable {
  /// Pointer to the function that should be invoked when this reference is
  /// released.
  void (*invalidate)(struct HermesABIManagedPointer *self);
};
struct HermesABIManagedPointer {
  const struct HermesABIManagedPointerVTable *vtable;
};

/// Enum for the types of errors that may be returned. These also indicate how
/// the error information should be retrieved.
enum HermesABIErrorCode {
  HermesABIErrorCodeNativeException,
  HermesABIErrorCodeJSError,
};

#define HERMES_ABI_POINTER_TYPES(V) \
  V(Object)                         \
  V(Array)                          \
  V(String)                         \
  V(BigInt)                         \
  V(Symbol)                         \
  V(Function)                       \
  V(ArrayBuffer)                    \
  V(PropNameID)                     \
  V(WeakObject)

/// For each type of pointer reference that can be held across the ABI, define
/// two structs. The first just wraps a HermesABIManagedPointer * to indicate
/// the type it references. The second allows us to represent a value that is
/// either a pointer or an error, and packs the error code such that the struct
/// is still pointer sized. This works by using the low bit of the pointer to
/// indicate that there is an error, since we know that the pointer is aligned
/// to the word size.
/// The second lowest bit is reserved for future use. If the low bit is set, the
/// error code can be obtained by right shifting ptr_or_error by 2.

#define DECLARE_HERMES_ABI_POINTER_TYPE(name) \
  struct HermesABI##name {                    \
    struct HermesABIManagedPointer *pointer;  \
  };                                          \
  struct HermesABI##name##OrError {           \
    uintptr_t ptr_or_error;                   \
  };

HERMES_ABI_POINTER_TYPES(DECLARE_HERMES_ABI_POINTER_TYPE)
#undef DECLARE_HERMES_ABI_POINTER_TYPE

/// Define the return type for functions that may return void or an error code.
/// This uses the same scheme as pointers, where the low bit indicates whether
/// there was an error, and the remaining bits hold the error code.
struct HermesABIVoidOrError {
  uintptr_t void_or_error;
};

/// Define a struct for holding a boolean value. Similar to the above, the low
/// bit is used to indicate whether there was an error, and the remaining bits
/// hold either the boolean value or the error code.
struct HermesABIBoolOrError {
  uintptr_t bool_or_error;
};

/// Define a struct for holding either a uint8_t* or an error code. Note that
/// this requires a separate field to disambiguate errors, since there are no
/// alignment bits available in the pointer.
struct HermesABIUint8PtrOrError {
  bool is_error;
  union {
    uint8_t *val;
    uint16_t error;
  } data;
};

/// Define a struct for holding either a size_t or an error code.
struct HermesABISizeTOrError {
  bool is_error;
  union {
    size_t val;
    uint16_t error;
  } data;
};

/// Similar to the pointer types, PropNameIDListPtr is known to always point to
/// a word aligned type, so we can pack the error message using the same
/// scheme.
struct HermesABIPropNameIDListPtrOrError {
  uintptr_t ptr_or_error;
};

/// Always set the top bit for pointers so they can be easily checked.
#define HERMES_ABI_POINTER_MASK (1 << (sizeof(int) * 8 - 1))

/// Enum for the types of JavaScript values that can be represented in the ABI.
enum HermesABIValueKind {
  HermesABIValueKindUndefined = 0,
  HermesABIValueKindNull = 1,
  HermesABIValueKindBoolean = 2,
  HermesABIValueKindError = 3,
  HermesABIValueKindNumber = 4,
  HermesABIValueKindSymbol = 5 | HERMES_ABI_POINTER_MASK,
  HermesABIValueKindBigInt = 6 | HERMES_ABI_POINTER_MASK,
  HermesABIValueKindString = 7 | HERMES_ABI_POINTER_MASK,
  HermesABIValueKindObject = 9 | HERMES_ABI_POINTER_MASK,
};

/// Struct representing a JavaScript value. This owns the reference to any
/// HermesABIManagedPointer, and must be explicitly released when no longer
/// needed. For efficiency, the error tag and code are part of the
/// representation, but this type should never be used when an error is
/// possible, use HermesABIValueOrError instead.
struct HermesABIValue {
  enum HermesABIValueKind kind;
  union {
    bool boolean;
    double number;
    struct HermesABIManagedPointer *pointer;
    enum HermesABIErrorCode error;
  } data;
};

/// Struct for representing either a HermesABIValue or an error. The underlying
/// representation is exactly the same as HermesABIValue, so this is purely to
/// provide type safety.
struct HermesABIValueOrError {
  struct HermesABIValue value;
};

/// Define a growable byte buffer that can be used to pass binary data and
/// strings. This allows the user of the C-API to wrap their own resizable
/// buffer and provide it to the API implementation so that data of variable
/// length can be passed without requiring an additional copy.
/// For example, writing to the buffer is typically done as follows:
///   if (buf->size < numBytes) {
///     buf->vtable->try_grow_to(buf, numBytes);
///     if (buf->size < numBytes)
///       fatal("Failed to allocate memory");
///   }
///   memcpy(buf->data, data, numBytes);
///   buf->used = numBytes;
struct HermesABIGrowableBufferVTable {
  /// Grow the buffer to the specified size. It may not acquire the full
  /// amount, so a caller should check the new size. This can only be used to
  /// grow the buffer, values smaller than the current size will have no effect.
  void (*try_grow_to)(struct HermesABIGrowableBuffer *buf, size_t sz);
};
struct HermesABIGrowableBuffer {
  const struct HermesABIGrowableBufferVTable *vtable;
  /// The current pointer to the buffer data. This may be updated by a call to
  /// try_grow_to.
  uint8_t *data;
  /// The total size of the buffer in bytes.
  size_t size;
  /// The number of bytes currently used.
  size_t used;
};

/// Define the structure for buffers containing JS source or bytecode. This is
/// designed to mirror the functionality of jsi::Buffer.
struct HermesABIBufferVTable {
  void (*release)(struct HermesABIBuffer *self);
};
struct HermesABIBuffer {
  const struct HermesABIBufferVTable *vtable;
  const uint8_t *data;
  size_t size;
};

struct HermesABIRuntimeVTable {
  /// Release the given runtime.
  void (*release)(struct HermesABIRuntime *);

  /// Methods for retrieving and clearing exceptions. An exception should be
  /// retrieved if and only if some method returned an error value.
  /// Get and clear the stored JS exception value. This should be called exactly
  /// once after an exception is thrown.
  struct HermesABIValue (*get_and_clear_js_error_value)(
      struct HermesABIRuntime *rt);
  /// Get and clear the stored native exception message. The message is UTF-8
  /// encoded.
  void (*get_and_clear_native_exception_message)(
      struct HermesABIRuntime *rt,
      struct HermesABIGrowableBuffer *msg_buf);

  /// Set the current error before returning control to the ABI. These are
  /// intended to be used to throw exceptions from HostFunctions and
  /// HostObjects.
  /// Report a JavaScript exception with the given value.
  void (*set_js_error_value)(
      struct HermesABIRuntime *rt,
      const struct HermesABIValue *error_value);
  /// Report a native exception with the given UTF-8 message.
  void (*set_native_exception_message)(
      struct HermesABIRuntime *rt,
      const uint8_t *utf8,
      size_t length);

  struct HermesABIPropNameID (*clone_propnameid)(
      struct HermesABIRuntime *rt,
      struct HermesABIPropNameID name);
  struct HermesABIString (
      *clone_string)(struct HermesABIRuntime *rt, struct HermesABIString str);
  struct HermesABISymbol (
      *clone_symbol)(struct HermesABIRuntime *rt, struct HermesABISymbol sym);
  struct HermesABIObject (
      *clone_object)(struct HermesABIRuntime *rt, struct HermesABIObject obj);
  struct HermesABIBigInt (*clone_bigint)(
      struct HermesABIRuntime *rt,
      struct HermesABIBigInt bigint);

  /// Evaluate the given JavaScript source with an associated source URL in the
  /// given runtime, and return the result. The buffer must have a past-the-end
  /// null terminator.
  struct HermesABIValueOrError (*evaluate_javascript_source)(
      struct HermesABIRuntime *rt,
      struct HermesABIBuffer *buf,
      const char *source_url,
      size_t source_url_len);

  /// Evaluate the given Hermes bytecode with an associated source URL in the
  /// given runtime, and return the result. No validation is performed on the
  /// bytecode, so the caller must ensure it is valid.
  struct HermesABIValueOrError (*evaluate_hermes_bytecode)(
      struct HermesABIRuntime *rt,
      struct HermesABIBuffer *buf,
      const char *source_url,
      size_t source_url_len);
};

/// An instance of a Hermes Runtime.
struct HermesABIRuntime {
  const struct HermesABIRuntimeVTable *vt;
};

struct HermesABIVTable {
  /// Create a new instance of a Hermes Runtime, and return a pointer to it. The
  /// runtime must be explicitly released when it is no longer needed.
  struct HermesABIRuntime *(*make_hermes_runtime)(
      const struct HermesABIRuntimeConfig *config);

  /// Check if the given buffer contains Hermes bytecode.
  bool (*is_hermes_bytecode)(const uint8_t *buf, size_t len);
};

#endif