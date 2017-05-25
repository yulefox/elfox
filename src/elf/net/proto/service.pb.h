// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: proto/service.proto

#ifndef PROTOBUF_proto_2fservice_2eproto__INCLUDED
#define PROTOBUF_proto_2fservice_2eproto__INCLUDED

#include <string>

#include <google/protobuf/stubs/common.h>

#if GOOGLE_PROTOBUF_VERSION < 3002000
#error This file was generated by a newer version of protoc which is
#error incompatible with your Protocol Buffer headers.  Please update
#error your headers.
#endif
#if 3002000 < GOOGLE_PROTOBUF_MIN_PROTOC_VERSION
#error This file was generated by an older version of protoc which is
#error incompatible with your Protocol Buffer headers.  Please
#error regenerate this file with a newer version of protoc.
#endif

#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/arena.h>
#include <google/protobuf/arenastring.h>
#include <google/protobuf/generated_message_util.h>
#include <google/protobuf/metadata.h>
#include <google/protobuf/message.h>
#include <google/protobuf/repeated_field.h>  // IWYU pragma: export
#include <google/protobuf/extension_set.h>  // IWYU pragma: export
#include <google/protobuf/unknown_field_set.h>
// @@protoc_insertion_point(includes)
namespace proto {
class Packet;
class PacketDefaultTypeInternal;
extern PacketDefaultTypeInternal _Packet_default_instance_;
}  // namespace proto

namespace proto {

namespace protobuf_proto_2fservice_2eproto {
// Internal implementation detail -- do not call these.
struct TableStruct {
  static const ::google::protobuf::uint32 offsets[];
  static void InitDefaultsImpl();
  static void Shutdown();
};
void AddDescriptors();
void InitDefaults();
}  // namespace protobuf_proto_2fservice_2eproto

// ===================================================================

class Packet : public ::google::protobuf::Message /* @@protoc_insertion_point(class_definition:proto.Packet) */ {
 public:
  Packet();
  virtual ~Packet();

  Packet(const Packet& from);

  inline Packet& operator=(const Packet& from) {
    CopyFrom(from);
    return *this;
  }

  static const ::google::protobuf::Descriptor* descriptor();
  static const Packet& default_instance();

  static inline const Packet* internal_default_instance() {
    return reinterpret_cast<const Packet*>(
               &_Packet_default_instance_);
  }

  void Swap(Packet* other);

  // implements Message ----------------------------------------------

  inline Packet* New() const PROTOBUF_FINAL { return New(NULL); }

  Packet* New(::google::protobuf::Arena* arena) const PROTOBUF_FINAL;
  void CopyFrom(const ::google::protobuf::Message& from) PROTOBUF_FINAL;
  void MergeFrom(const ::google::protobuf::Message& from) PROTOBUF_FINAL;
  void CopyFrom(const Packet& from);
  void MergeFrom(const Packet& from);
  void Clear() PROTOBUF_FINAL;
  bool IsInitialized() const PROTOBUF_FINAL;

  size_t ByteSizeLong() const PROTOBUF_FINAL;
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input) PROTOBUF_FINAL;
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const PROTOBUF_FINAL;
  ::google::protobuf::uint8* InternalSerializeWithCachedSizesToArray(
      bool deterministic, ::google::protobuf::uint8* target) const PROTOBUF_FINAL;
  ::google::protobuf::uint8* SerializeWithCachedSizesToArray(::google::protobuf::uint8* output)
      const PROTOBUF_FINAL {
    return InternalSerializeWithCachedSizesToArray(
        ::google::protobuf::io::CodedOutputStream::IsDefaultSerializationDeterministic(), output);
  }
  int GetCachedSize() const PROTOBUF_FINAL { return _cached_size_; }
  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const PROTOBUF_FINAL;
  void InternalSwap(Packet* other);
  private:
  inline ::google::protobuf::Arena* GetArenaNoVirtual() const {
    return NULL;
  }
  inline void* MaybeArenaPtr() const {
    return NULL;
  }
  public:

  ::google::protobuf::Metadata GetMetadata() const PROTOBUF_FINAL;

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  // string type = 1;
  void clear_type();
  static const int kTypeFieldNumber = 1;
  const ::std::string& type() const;
  void set_type(const ::std::string& value);
  #if LANG_CXX11
  void set_type(::std::string&& value);
  #endif
  void set_type(const char* value);
  void set_type(const char* value, size_t size);
  ::std::string* mutable_type();
  ::std::string* release_type();
  void set_allocated_type(::std::string* type);

  // bytes payload = 2;
  void clear_payload();
  static const int kPayloadFieldNumber = 2;
  const ::std::string& payload() const;
  void set_payload(const ::std::string& value);
  #if LANG_CXX11
  void set_payload(::std::string&& value);
  #endif
  void set_payload(const char* value);
  void set_payload(const void* value, size_t size);
  ::std::string* mutable_payload();
  ::std::string* release_payload();
  void set_allocated_payload(::std::string* payload);

  // @@protoc_insertion_point(class_scope:proto.Packet)
 private:

  ::google::protobuf::internal::InternalMetadataWithArena _internal_metadata_;
  ::google::protobuf::internal::ArenaStringPtr type_;
  ::google::protobuf::internal::ArenaStringPtr payload_;
  mutable int _cached_size_;
  friend struct  protobuf_proto_2fservice_2eproto::TableStruct;
};
// ===================================================================


// ===================================================================

#if !PROTOBUF_INLINE_NOT_IN_HEADERS
// Packet

// string type = 1;
inline void Packet::clear_type() {
  type_.ClearToEmptyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline const ::std::string& Packet::type() const {
  // @@protoc_insertion_point(field_get:proto.Packet.type)
  return type_.GetNoArena();
}
inline void Packet::set_type(const ::std::string& value) {
  
  type_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), value);
  // @@protoc_insertion_point(field_set:proto.Packet.type)
}
#if LANG_CXX11
inline void Packet::set_type(::std::string&& value) {
  
  type_.SetNoArena(
    &::google::protobuf::internal::GetEmptyStringAlreadyInited(), std::move(value));
  // @@protoc_insertion_point(field_set_rvalue:proto.Packet.type)
}
#endif
inline void Packet::set_type(const char* value) {
  
  type_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::string(value));
  // @@protoc_insertion_point(field_set_char:proto.Packet.type)
}
inline void Packet::set_type(const char* value, size_t size) {
  
  type_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(),
      ::std::string(reinterpret_cast<const char*>(value), size));
  // @@protoc_insertion_point(field_set_pointer:proto.Packet.type)
}
inline ::std::string* Packet::mutable_type() {
  
  // @@protoc_insertion_point(field_mutable:proto.Packet.type)
  return type_.MutableNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline ::std::string* Packet::release_type() {
  // @@protoc_insertion_point(field_release:proto.Packet.type)
  
  return type_.ReleaseNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void Packet::set_allocated_type(::std::string* type) {
  if (type != NULL) {
    
  } else {
    
  }
  type_.SetAllocatedNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), type);
  // @@protoc_insertion_point(field_set_allocated:proto.Packet.type)
}

// bytes payload = 2;
inline void Packet::clear_payload() {
  payload_.ClearToEmptyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline const ::std::string& Packet::payload() const {
  // @@protoc_insertion_point(field_get:proto.Packet.payload)
  return payload_.GetNoArena();
}
inline void Packet::set_payload(const ::std::string& value) {
  
  payload_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), value);
  // @@protoc_insertion_point(field_set:proto.Packet.payload)
}
#if LANG_CXX11
inline void Packet::set_payload(::std::string&& value) {
  
  payload_.SetNoArena(
    &::google::protobuf::internal::GetEmptyStringAlreadyInited(), std::move(value));
  // @@protoc_insertion_point(field_set_rvalue:proto.Packet.payload)
}
#endif
inline void Packet::set_payload(const char* value) {
  
  payload_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::string(value));
  // @@protoc_insertion_point(field_set_char:proto.Packet.payload)
}
inline void Packet::set_payload(const void* value, size_t size) {
  
  payload_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(),
      ::std::string(reinterpret_cast<const char*>(value), size));
  // @@protoc_insertion_point(field_set_pointer:proto.Packet.payload)
}
inline ::std::string* Packet::mutable_payload() {
  
  // @@protoc_insertion_point(field_mutable:proto.Packet.payload)
  return payload_.MutableNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline ::std::string* Packet::release_payload() {
  // @@protoc_insertion_point(field_release:proto.Packet.payload)
  
  return payload_.ReleaseNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void Packet::set_allocated_payload(::std::string* payload) {
  if (payload != NULL) {
    
  } else {
    
  }
  payload_.SetAllocatedNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), payload);
  // @@protoc_insertion_point(field_set_allocated:proto.Packet.payload)
}

#endif  // !PROTOBUF_INLINE_NOT_IN_HEADERS

// @@protoc_insertion_point(namespace_scope)


}  // namespace proto

// @@protoc_insertion_point(global_scope)

#endif  // PROTOBUF_proto_2fservice_2eproto__INCLUDED