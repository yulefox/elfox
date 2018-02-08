// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: proto/dbus.proto

#ifndef PROTOBUF_proto_2fdbus_2eproto__INCLUDED
#define PROTOBUF_proto_2fdbus_2eproto__INCLUDED

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
namespace pb {
class Packet;
class PacketDefaultTypeInternal;
extern PacketDefaultTypeInternal _Packet_default_instance_;
class Peer;
class PeerDefaultTypeInternal;
extern PeerDefaultTypeInternal _Peer_default_instance_;
}  // namespace pb

namespace pb {

namespace protobuf_proto_2fdbus_2eproto {
// Internal implementation detail -- do not call these.
struct TableStruct {
  static const ::google::protobuf::uint32 offsets[];
  static void InitDefaultsImpl();
  static void Shutdown();
};
void AddDescriptors();
void InitDefaults();
}  // namespace protobuf_proto_2fdbus_2eproto

// ===================================================================

class Peer : public ::google::protobuf::Message /* @@protoc_insertion_point(class_definition:pb.Peer) */ {
 public:
  Peer();
  virtual ~Peer();

  Peer(const Peer& from);

  inline Peer& operator=(const Peer& from) {
    CopyFrom(from);
    return *this;
  }

  static const ::google::protobuf::Descriptor* descriptor();
  static const Peer& default_instance();

  static inline const Peer* internal_default_instance() {
    return reinterpret_cast<const Peer*>(
               &_Peer_default_instance_);
  }

  void Swap(Peer* other);

  // implements Message ----------------------------------------------

  inline Peer* New() const PROTOBUF_FINAL { return New(NULL); }

  Peer* New(::google::protobuf::Arena* arena) const PROTOBUF_FINAL;
  void CopyFrom(const ::google::protobuf::Message& from) PROTOBUF_FINAL;
  void MergeFrom(const ::google::protobuf::Message& from) PROTOBUF_FINAL;
  void CopyFrom(const Peer& from);
  void MergeFrom(const Peer& from);
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
  void InternalSwap(Peer* other);
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

  // repeated int64 peers = 2;
  int peers_size() const;
  void clear_peers();
  static const int kPeersFieldNumber = 2;
  ::google::protobuf::int64 peers(int index) const;
  void set_peers(int index, ::google::protobuf::int64 value);
  void add_peers(::google::protobuf::int64 value);
  const ::google::protobuf::RepeatedField< ::google::protobuf::int64 >&
      peers() const;
  ::google::protobuf::RepeatedField< ::google::protobuf::int64 >*
      mutable_peers();

  // string name = 1;
  void clear_name();
  static const int kNameFieldNumber = 1;
  const ::std::string& name() const;
  void set_name(const ::std::string& value);
  #if LANG_CXX11
  void set_name(::std::string&& value);
  #endif
  void set_name(const char* value);
  void set_name(const char* value, size_t size);
  ::std::string* mutable_name();
  ::std::string* release_name();
  void set_allocated_name(::std::string* name);

  // @@protoc_insertion_point(class_scope:pb.Peer)
 private:

  ::google::protobuf::internal::InternalMetadataWithArena _internal_metadata_;
  ::google::protobuf::RepeatedField< ::google::protobuf::int64 > peers_;
  mutable int _peers_cached_byte_size_;
  ::google::protobuf::internal::ArenaStringPtr name_;
  mutable int _cached_size_;
  friend struct  protobuf_proto_2fdbus_2eproto::TableStruct;
};
// -------------------------------------------------------------------

class Packet : public ::google::protobuf::Message /* @@protoc_insertion_point(class_definition:pb.Packet) */ {
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

  // string type = 8;
  void clear_type();
  static const int kTypeFieldNumber = 8;
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

  // bytes payload = 9;
  void clear_payload();
  static const int kPayloadFieldNumber = 9;
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

  // .pb.Peer peer = 1;
  bool has_peer() const;
  void clear_peer();
  static const int kPeerFieldNumber = 1;
  const ::pb::Peer& peer() const;
  ::pb::Peer* mutable_peer();
  ::pb::Peer* release_peer();
  void set_allocated_peer(::pb::Peer* peer);

  // @@protoc_insertion_point(class_scope:pb.Packet)
 private:

  ::google::protobuf::internal::InternalMetadataWithArena _internal_metadata_;
  ::google::protobuf::internal::ArenaStringPtr type_;
  ::google::protobuf::internal::ArenaStringPtr payload_;
  ::pb::Peer* peer_;
  mutable int _cached_size_;
  friend struct  protobuf_proto_2fdbus_2eproto::TableStruct;
};
// ===================================================================


// ===================================================================

#if !PROTOBUF_INLINE_NOT_IN_HEADERS
// Peer

// string name = 1;
inline void Peer::clear_name() {
  name_.ClearToEmptyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline const ::std::string& Peer::name() const {
  // @@protoc_insertion_point(field_get:pb.Peer.name)
  return name_.GetNoArena();
}
inline void Peer::set_name(const ::std::string& value) {
  
  name_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), value);
  // @@protoc_insertion_point(field_set:pb.Peer.name)
}
#if LANG_CXX11
inline void Peer::set_name(::std::string&& value) {
  
  name_.SetNoArena(
    &::google::protobuf::internal::GetEmptyStringAlreadyInited(), std::move(value));
  // @@protoc_insertion_point(field_set_rvalue:pb.Peer.name)
}
#endif
inline void Peer::set_name(const char* value) {
  
  name_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::string(value));
  // @@protoc_insertion_point(field_set_char:pb.Peer.name)
}
inline void Peer::set_name(const char* value, size_t size) {
  
  name_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(),
      ::std::string(reinterpret_cast<const char*>(value), size));
  // @@protoc_insertion_point(field_set_pointer:pb.Peer.name)
}
inline ::std::string* Peer::mutable_name() {
  
  // @@protoc_insertion_point(field_mutable:pb.Peer.name)
  return name_.MutableNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline ::std::string* Peer::release_name() {
  // @@protoc_insertion_point(field_release:pb.Peer.name)
  
  return name_.ReleaseNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void Peer::set_allocated_name(::std::string* name) {
  if (name != NULL) {
    
  } else {
    
  }
  name_.SetAllocatedNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), name);
  // @@protoc_insertion_point(field_set_allocated:pb.Peer.name)
}

// repeated int64 peers = 2;
inline int Peer::peers_size() const {
  return peers_.size();
}
inline void Peer::clear_peers() {
  peers_.Clear();
}
inline ::google::protobuf::int64 Peer::peers(int index) const {
  // @@protoc_insertion_point(field_get:pb.Peer.peers)
  return peers_.Get(index);
}
inline void Peer::set_peers(int index, ::google::protobuf::int64 value) {
  peers_.Set(index, value);
  // @@protoc_insertion_point(field_set:pb.Peer.peers)
}
inline void Peer::add_peers(::google::protobuf::int64 value) {
  peers_.Add(value);
  // @@protoc_insertion_point(field_add:pb.Peer.peers)
}
inline const ::google::protobuf::RepeatedField< ::google::protobuf::int64 >&
Peer::peers() const {
  // @@protoc_insertion_point(field_list:pb.Peer.peers)
  return peers_;
}
inline ::google::protobuf::RepeatedField< ::google::protobuf::int64 >*
Peer::mutable_peers() {
  // @@protoc_insertion_point(field_mutable_list:pb.Peer.peers)
  return &peers_;
}

// -------------------------------------------------------------------

// Packet

// .pb.Peer peer = 1;
inline bool Packet::has_peer() const {
  return this != internal_default_instance() && peer_ != NULL;
}
inline void Packet::clear_peer() {
  if (GetArenaNoVirtual() == NULL && peer_ != NULL) delete peer_;
  peer_ = NULL;
}
inline const ::pb::Peer& Packet::peer() const {
  // @@protoc_insertion_point(field_get:pb.Packet.peer)
  return peer_ != NULL ? *peer_
                         : *::pb::Peer::internal_default_instance();
}
inline ::pb::Peer* Packet::mutable_peer() {
  
  if (peer_ == NULL) {
    peer_ = new ::pb::Peer;
  }
  // @@protoc_insertion_point(field_mutable:pb.Packet.peer)
  return peer_;
}
inline ::pb::Peer* Packet::release_peer() {
  // @@protoc_insertion_point(field_release:pb.Packet.peer)
  
  ::pb::Peer* temp = peer_;
  peer_ = NULL;
  return temp;
}
inline void Packet::set_allocated_peer(::pb::Peer* peer) {
  delete peer_;
  peer_ = peer;
  if (peer) {
    
  } else {
    
  }
  // @@protoc_insertion_point(field_set_allocated:pb.Packet.peer)
}

// string type = 8;
inline void Packet::clear_type() {
  type_.ClearToEmptyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline const ::std::string& Packet::type() const {
  // @@protoc_insertion_point(field_get:pb.Packet.type)
  return type_.GetNoArena();
}
inline void Packet::set_type(const ::std::string& value) {
  
  type_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), value);
  // @@protoc_insertion_point(field_set:pb.Packet.type)
}
#if LANG_CXX11
inline void Packet::set_type(::std::string&& value) {
  
  type_.SetNoArena(
    &::google::protobuf::internal::GetEmptyStringAlreadyInited(), std::move(value));
  // @@protoc_insertion_point(field_set_rvalue:pb.Packet.type)
}
#endif
inline void Packet::set_type(const char* value) {
  
  type_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::string(value));
  // @@protoc_insertion_point(field_set_char:pb.Packet.type)
}
inline void Packet::set_type(const char* value, size_t size) {
  
  type_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(),
      ::std::string(reinterpret_cast<const char*>(value), size));
  // @@protoc_insertion_point(field_set_pointer:pb.Packet.type)
}
inline ::std::string* Packet::mutable_type() {
  
  // @@protoc_insertion_point(field_mutable:pb.Packet.type)
  return type_.MutableNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline ::std::string* Packet::release_type() {
  // @@protoc_insertion_point(field_release:pb.Packet.type)
  
  return type_.ReleaseNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void Packet::set_allocated_type(::std::string* type) {
  if (type != NULL) {
    
  } else {
    
  }
  type_.SetAllocatedNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), type);
  // @@protoc_insertion_point(field_set_allocated:pb.Packet.type)
}

// bytes payload = 9;
inline void Packet::clear_payload() {
  payload_.ClearToEmptyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline const ::std::string& Packet::payload() const {
  // @@protoc_insertion_point(field_get:pb.Packet.payload)
  return payload_.GetNoArena();
}
inline void Packet::set_payload(const ::std::string& value) {
  
  payload_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), value);
  // @@protoc_insertion_point(field_set:pb.Packet.payload)
}
#if LANG_CXX11
inline void Packet::set_payload(::std::string&& value) {
  
  payload_.SetNoArena(
    &::google::protobuf::internal::GetEmptyStringAlreadyInited(), std::move(value));
  // @@protoc_insertion_point(field_set_rvalue:pb.Packet.payload)
}
#endif
inline void Packet::set_payload(const char* value) {
  
  payload_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::string(value));
  // @@protoc_insertion_point(field_set_char:pb.Packet.payload)
}
inline void Packet::set_payload(const void* value, size_t size) {
  
  payload_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(),
      ::std::string(reinterpret_cast<const char*>(value), size));
  // @@protoc_insertion_point(field_set_pointer:pb.Packet.payload)
}
inline ::std::string* Packet::mutable_payload() {
  
  // @@protoc_insertion_point(field_mutable:pb.Packet.payload)
  return payload_.MutableNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline ::std::string* Packet::release_payload() {
  // @@protoc_insertion_point(field_release:pb.Packet.payload)
  
  return payload_.ReleaseNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void Packet::set_allocated_payload(::std::string* payload) {
  if (payload != NULL) {
    
  } else {
    
  }
  payload_.SetAllocatedNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), payload);
  // @@protoc_insertion_point(field_set_allocated:pb.Packet.payload)
}

#endif  // !PROTOBUF_INLINE_NOT_IN_HEADERS
// -------------------------------------------------------------------


// @@protoc_insertion_point(namespace_scope)


}  // namespace pb

// @@protoc_insertion_point(global_scope)

#endif  // PROTOBUF_proto_2fdbus_2eproto__INCLUDED