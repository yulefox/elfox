// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: proto/dbus.proto

#define INTERNAL_SUPPRESS_PROTOBUF_FIELD_DEPRECATION
#include "proto/dbus.pb.h"

#include <algorithm>

#include <google/protobuf/stubs/common.h>
#include <google/protobuf/stubs/port.h>
#include <google/protobuf/stubs/once.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/wire_format_lite_inl.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/generated_message_reflection.h>
#include <google/protobuf/reflection_ops.h>
#include <google/protobuf/wire_format.h>
// @@protoc_insertion_point(includes)

namespace pb {
class PeerDefaultTypeInternal : public ::google::protobuf::internal::ExplicitlyConstructed<Peer> {
} _Peer_default_instance_;
class PacketDefaultTypeInternal : public ::google::protobuf::internal::ExplicitlyConstructed<Packet> {
} _Packet_default_instance_;

namespace protobuf_proto_2fdbus_2eproto {


namespace {

::google::protobuf::Metadata file_level_metadata[2];

}  // namespace

const ::google::protobuf::uint32 TableStruct::offsets[] = {
  ~0u,  // no _has_bits_
  GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(Peer, _internal_metadata_),
  ~0u,  // no _extensions_
  ~0u,  // no _oneof_case_
  GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(Peer, name_),
  GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(Peer, peers_),
  ~0u,  // no _has_bits_
  GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(Packet, _internal_metadata_),
  ~0u,  // no _extensions_
  ~0u,  // no _oneof_case_
  GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(Packet, peer_),
  GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(Packet, type_),
  GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(Packet, payload_),
};

static const ::google::protobuf::internal::MigrationSchema schemas[] = {
  { 0, -1, sizeof(Peer)},
  { 6, -1, sizeof(Packet)},
};

static ::google::protobuf::Message const * const file_default_instances[] = {
  reinterpret_cast<const ::google::protobuf::Message*>(&_Peer_default_instance_),
  reinterpret_cast<const ::google::protobuf::Message*>(&_Packet_default_instance_),
};

namespace {

void protobuf_AssignDescriptors() {
  AddDescriptors();
  ::google::protobuf::MessageFactory* factory = NULL;
  AssignDescriptors(
      "proto/dbus.proto", schemas, file_default_instances, TableStruct::offsets, factory,
      file_level_metadata, NULL, NULL);
}

void protobuf_AssignDescriptorsOnce() {
  static GOOGLE_PROTOBUF_DECLARE_ONCE(once);
  ::google::protobuf::GoogleOnceInit(&once, &protobuf_AssignDescriptors);
}

void protobuf_RegisterTypes(const ::std::string&) GOOGLE_ATTRIBUTE_COLD;
void protobuf_RegisterTypes(const ::std::string&) {
  protobuf_AssignDescriptorsOnce();
  ::google::protobuf::internal::RegisterAllTypes(file_level_metadata, 2);
}

}  // namespace

void TableStruct::Shutdown() {
  _Peer_default_instance_.Shutdown();
  delete file_level_metadata[0].reflection;
  _Packet_default_instance_.Shutdown();
  delete file_level_metadata[1].reflection;
}

void TableStruct::InitDefaultsImpl() {
  GOOGLE_PROTOBUF_VERIFY_VERSION;

  ::google::protobuf::internal::InitProtobufDefaults();
  _Peer_default_instance_.DefaultConstruct();
  _Packet_default_instance_.DefaultConstruct();
  _Packet_default_instance_.get_mutable()->peer_ = const_cast< ::pb::Peer*>(
      ::pb::Peer::internal_default_instance());
}

void InitDefaults() {
  static GOOGLE_PROTOBUF_DECLARE_ONCE(once);
  ::google::protobuf::GoogleOnceInit(&once, &TableStruct::InitDefaultsImpl);
}
void AddDescriptorsImpl() {
  InitDefaults();
  static const char descriptor[] = {
      "\n\020proto/dbus.proto\022\002pb\"#\n\004Peer\022\014\n\004name\030\001"
      " \001(\t\022\r\n\005peers\030\002 \003(\003\"\?\n\006Packet\022\026\n\004peer\030\001 "
      "\001(\0132\010.pb.Peer\022\014\n\004type\030\010 \001(\t\022\017\n\007payload\030\t"
      " \001(\0142,\n\004DBus\022$\n\006Stream\022\n.pb.Packet\032\n.pb."
      "Packet(\0010\001b\006proto3"
  };
  ::google::protobuf::DescriptorPool::InternalAddGeneratedFile(
      descriptor, 178);
  ::google::protobuf::MessageFactory::InternalRegisterGeneratedFile(
    "proto/dbus.proto", &protobuf_RegisterTypes);
  ::google::protobuf::internal::OnShutdown(&TableStruct::Shutdown);
}

void AddDescriptors() {
  static GOOGLE_PROTOBUF_DECLARE_ONCE(once);
  ::google::protobuf::GoogleOnceInit(&once, &AddDescriptorsImpl);
}
// Force AddDescriptors() to be called at static initialization time.
struct StaticDescriptorInitializer {
  StaticDescriptorInitializer() {
    AddDescriptors();
  }
} static_descriptor_initializer;

}  // namespace protobuf_proto_2fdbus_2eproto


// ===================================================================

#if !defined(_MSC_VER) || _MSC_VER >= 1900
const int Peer::kNameFieldNumber;
const int Peer::kPeersFieldNumber;
#endif  // !defined(_MSC_VER) || _MSC_VER >= 1900

Peer::Peer()
  : ::google::protobuf::Message(), _internal_metadata_(NULL) {
  if (GOOGLE_PREDICT_TRUE(this != internal_default_instance())) {
    protobuf_proto_2fdbus_2eproto::InitDefaults();
  }
  SharedCtor();
  // @@protoc_insertion_point(constructor:pb.Peer)
}
Peer::Peer(const Peer& from)
  : ::google::protobuf::Message(),
      _internal_metadata_(NULL),
      peers_(from.peers_),
      _cached_size_(0) {
  _internal_metadata_.MergeFrom(from._internal_metadata_);
  name_.UnsafeSetDefault(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
  if (from.name().size() > 0) {
    name_.AssignWithDefault(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), from.name_);
  }
  // @@protoc_insertion_point(copy_constructor:pb.Peer)
}

void Peer::SharedCtor() {
  name_.UnsafeSetDefault(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
  _cached_size_ = 0;
}

Peer::~Peer() {
  // @@protoc_insertion_point(destructor:pb.Peer)
  SharedDtor();
}

void Peer::SharedDtor() {
  name_.DestroyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}

void Peer::SetCachedSize(int size) const {
  GOOGLE_SAFE_CONCURRENT_WRITES_BEGIN();
  _cached_size_ = size;
  GOOGLE_SAFE_CONCURRENT_WRITES_END();
}
const ::google::protobuf::Descriptor* Peer::descriptor() {
  protobuf_proto_2fdbus_2eproto::protobuf_AssignDescriptorsOnce();
  return protobuf_proto_2fdbus_2eproto::file_level_metadata[0].descriptor;
}

const Peer& Peer::default_instance() {
  protobuf_proto_2fdbus_2eproto::InitDefaults();
  return *internal_default_instance();
}

Peer* Peer::New(::google::protobuf::Arena* arena) const {
  Peer* n = new Peer;
  if (arena != NULL) {
    arena->Own(n);
  }
  return n;
}

void Peer::Clear() {
// @@protoc_insertion_point(message_clear_start:pb.Peer)
  peers_.Clear();
  name_.ClearToEmptyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}

bool Peer::MergePartialFromCodedStream(
    ::google::protobuf::io::CodedInputStream* input) {
#define DO_(EXPRESSION) if (!GOOGLE_PREDICT_TRUE(EXPRESSION)) goto failure
  ::google::protobuf::uint32 tag;
  // @@protoc_insertion_point(parse_start:pb.Peer)
  for (;;) {
    ::std::pair< ::google::protobuf::uint32, bool> p = input->ReadTagWithCutoffNoLastTag(127u);
    tag = p.first;
    if (!p.second) goto handle_unusual;
    switch (::google::protobuf::internal::WireFormatLite::GetTagFieldNumber(tag)) {
      // string name = 1;
      case 1: {
        if (tag == 10u) {
          DO_(::google::protobuf::internal::WireFormatLite::ReadString(
                input, this->mutable_name()));
          DO_(::google::protobuf::internal::WireFormatLite::VerifyUtf8String(
            this->name().data(), this->name().length(),
            ::google::protobuf::internal::WireFormatLite::PARSE,
            "pb.Peer.name"));
        } else {
          goto handle_unusual;
        }
        break;
      }

      // repeated int64 peers = 2;
      case 2: {
        if (tag == 18u) {
          DO_((::google::protobuf::internal::WireFormatLite::ReadPackedPrimitive<
                   ::google::protobuf::int64, ::google::protobuf::internal::WireFormatLite::TYPE_INT64>(
                 input, this->mutable_peers())));
        } else if (tag == 16u) {
          DO_((::google::protobuf::internal::WireFormatLite::ReadRepeatedPrimitiveNoInline<
                   ::google::protobuf::int64, ::google::protobuf::internal::WireFormatLite::TYPE_INT64>(
                 1, 18u, input, this->mutable_peers())));
        } else {
          goto handle_unusual;
        }
        break;
      }

      default: {
      handle_unusual:
        if (tag == 0 ||
            ::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==
            ::google::protobuf::internal::WireFormatLite::WIRETYPE_END_GROUP) {
          goto success;
        }
        DO_(::google::protobuf::internal::WireFormatLite::SkipField(input, tag));
        break;
      }
    }
  }
success:
  // @@protoc_insertion_point(parse_success:pb.Peer)
  return true;
failure:
  // @@protoc_insertion_point(parse_failure:pb.Peer)
  return false;
#undef DO_
}

void Peer::SerializeWithCachedSizes(
    ::google::protobuf::io::CodedOutputStream* output) const {
  // @@protoc_insertion_point(serialize_start:pb.Peer)
  // string name = 1;
  if (this->name().size() > 0) {
    ::google::protobuf::internal::WireFormatLite::VerifyUtf8String(
      this->name().data(), this->name().length(),
      ::google::protobuf::internal::WireFormatLite::SERIALIZE,
      "pb.Peer.name");
    ::google::protobuf::internal::WireFormatLite::WriteStringMaybeAliased(
      1, this->name(), output);
  }

  // repeated int64 peers = 2;
  if (this->peers_size() > 0) {
    ::google::protobuf::internal::WireFormatLite::WriteTag(2, ::google::protobuf::internal::WireFormatLite::WIRETYPE_LENGTH_DELIMITED, output);
    output->WriteVarint32(_peers_cached_byte_size_);
  }
  for (int i = 0; i < this->peers_size(); i++) {
    ::google::protobuf::internal::WireFormatLite::WriteInt64NoTag(
      this->peers(i), output);
  }

  // @@protoc_insertion_point(serialize_end:pb.Peer)
}

::google::protobuf::uint8* Peer::InternalSerializeWithCachedSizesToArray(
    bool deterministic, ::google::protobuf::uint8* target) const {
  (void)deterministic;  // Unused
  // @@protoc_insertion_point(serialize_to_array_start:pb.Peer)
  // string name = 1;
  if (this->name().size() > 0) {
    ::google::protobuf::internal::WireFormatLite::VerifyUtf8String(
      this->name().data(), this->name().length(),
      ::google::protobuf::internal::WireFormatLite::SERIALIZE,
      "pb.Peer.name");
    target =
      ::google::protobuf::internal::WireFormatLite::WriteStringToArray(
        1, this->name(), target);
  }

  // repeated int64 peers = 2;
  if (this->peers_size() > 0) {
    target = ::google::protobuf::internal::WireFormatLite::WriteTagToArray(
      2,
      ::google::protobuf::internal::WireFormatLite::WIRETYPE_LENGTH_DELIMITED,
      target);
    target = ::google::protobuf::io::CodedOutputStream::WriteVarint32ToArray(
      _peers_cached_byte_size_, target);
  }
  for (int i = 0; i < this->peers_size(); i++) {
    target = ::google::protobuf::internal::WireFormatLite::
      WriteInt64NoTagToArray(this->peers(i), target);
  }

  // @@protoc_insertion_point(serialize_to_array_end:pb.Peer)
  return target;
}

size_t Peer::ByteSizeLong() const {
// @@protoc_insertion_point(message_byte_size_start:pb.Peer)
  size_t total_size = 0;

  // repeated int64 peers = 2;
  {
    size_t data_size = ::google::protobuf::internal::WireFormatLite::
      Int64Size(this->peers_);
    if (data_size > 0) {
      total_size += 1 +
        ::google::protobuf::internal::WireFormatLite::Int32Size(data_size);
    }
    int cached_size = ::google::protobuf::internal::ToCachedSize(data_size);
    GOOGLE_SAFE_CONCURRENT_WRITES_BEGIN();
    _peers_cached_byte_size_ = cached_size;
    GOOGLE_SAFE_CONCURRENT_WRITES_END();
    total_size += data_size;
  }

  // string name = 1;
  if (this->name().size() > 0) {
    total_size += 1 +
      ::google::protobuf::internal::WireFormatLite::StringSize(
        this->name());
  }

  int cached_size = ::google::protobuf::internal::ToCachedSize(total_size);
  GOOGLE_SAFE_CONCURRENT_WRITES_BEGIN();
  _cached_size_ = cached_size;
  GOOGLE_SAFE_CONCURRENT_WRITES_END();
  return total_size;
}

void Peer::MergeFrom(const ::google::protobuf::Message& from) {
// @@protoc_insertion_point(generalized_merge_from_start:pb.Peer)
  GOOGLE_DCHECK_NE(&from, this);
  const Peer* source =
      ::google::protobuf::internal::DynamicCastToGenerated<const Peer>(
          &from);
  if (source == NULL) {
  // @@protoc_insertion_point(generalized_merge_from_cast_fail:pb.Peer)
    ::google::protobuf::internal::ReflectionOps::Merge(from, this);
  } else {
  // @@protoc_insertion_point(generalized_merge_from_cast_success:pb.Peer)
    MergeFrom(*source);
  }
}

void Peer::MergeFrom(const Peer& from) {
// @@protoc_insertion_point(class_specific_merge_from_start:pb.Peer)
  GOOGLE_DCHECK_NE(&from, this);
  _internal_metadata_.MergeFrom(from._internal_metadata_);
  peers_.MergeFrom(from.peers_);
  if (from.name().size() > 0) {

    name_.AssignWithDefault(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), from.name_);
  }
}

void Peer::CopyFrom(const ::google::protobuf::Message& from) {
// @@protoc_insertion_point(generalized_copy_from_start:pb.Peer)
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

void Peer::CopyFrom(const Peer& from) {
// @@protoc_insertion_point(class_specific_copy_from_start:pb.Peer)
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

bool Peer::IsInitialized() const {
  return true;
}

void Peer::Swap(Peer* other) {
  if (other == this) return;
  InternalSwap(other);
}
void Peer::InternalSwap(Peer* other) {
  peers_.UnsafeArenaSwap(&other->peers_);
  name_.Swap(&other->name_);
  std::swap(_cached_size_, other->_cached_size_);
}

::google::protobuf::Metadata Peer::GetMetadata() const {
  protobuf_proto_2fdbus_2eproto::protobuf_AssignDescriptorsOnce();
  return protobuf_proto_2fdbus_2eproto::file_level_metadata[0];
}

#if PROTOBUF_INLINE_NOT_IN_HEADERS
// Peer

// string name = 1;
void Peer::clear_name() {
  name_.ClearToEmptyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
const ::std::string& Peer::name() const {
  // @@protoc_insertion_point(field_get:pb.Peer.name)
  return name_.GetNoArena();
}
void Peer::set_name(const ::std::string& value) {
  
  name_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), value);
  // @@protoc_insertion_point(field_set:pb.Peer.name)
}
#if LANG_CXX11
void Peer::set_name(::std::string&& value) {
  
  name_.SetNoArena(
    &::google::protobuf::internal::GetEmptyStringAlreadyInited(), std::move(value));
  // @@protoc_insertion_point(field_set_rvalue:pb.Peer.name)
}
#endif
void Peer::set_name(const char* value) {
  
  name_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::string(value));
  // @@protoc_insertion_point(field_set_char:pb.Peer.name)
}
void Peer::set_name(const char* value, size_t size) {
  
  name_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(),
      ::std::string(reinterpret_cast<const char*>(value), size));
  // @@protoc_insertion_point(field_set_pointer:pb.Peer.name)
}
::std::string* Peer::mutable_name() {
  
  // @@protoc_insertion_point(field_mutable:pb.Peer.name)
  return name_.MutableNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
::std::string* Peer::release_name() {
  // @@protoc_insertion_point(field_release:pb.Peer.name)
  
  return name_.ReleaseNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
void Peer::set_allocated_name(::std::string* name) {
  if (name != NULL) {
    
  } else {
    
  }
  name_.SetAllocatedNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), name);
  // @@protoc_insertion_point(field_set_allocated:pb.Peer.name)
}

// repeated int64 peers = 2;
int Peer::peers_size() const {
  return peers_.size();
}
void Peer::clear_peers() {
  peers_.Clear();
}
::google::protobuf::int64 Peer::peers(int index) const {
  // @@protoc_insertion_point(field_get:pb.Peer.peers)
  return peers_.Get(index);
}
void Peer::set_peers(int index, ::google::protobuf::int64 value) {
  peers_.Set(index, value);
  // @@protoc_insertion_point(field_set:pb.Peer.peers)
}
void Peer::add_peers(::google::protobuf::int64 value) {
  peers_.Add(value);
  // @@protoc_insertion_point(field_add:pb.Peer.peers)
}
const ::google::protobuf::RepeatedField< ::google::protobuf::int64 >&
Peer::peers() const {
  // @@protoc_insertion_point(field_list:pb.Peer.peers)
  return peers_;
}
::google::protobuf::RepeatedField< ::google::protobuf::int64 >*
Peer::mutable_peers() {
  // @@protoc_insertion_point(field_mutable_list:pb.Peer.peers)
  return &peers_;
}

#endif  // PROTOBUF_INLINE_NOT_IN_HEADERS

// ===================================================================

#if !defined(_MSC_VER) || _MSC_VER >= 1900
const int Packet::kPeerFieldNumber;
const int Packet::kTypeFieldNumber;
const int Packet::kPayloadFieldNumber;
#endif  // !defined(_MSC_VER) || _MSC_VER >= 1900

Packet::Packet()
  : ::google::protobuf::Message(), _internal_metadata_(NULL) {
  if (GOOGLE_PREDICT_TRUE(this != internal_default_instance())) {
    protobuf_proto_2fdbus_2eproto::InitDefaults();
  }
  SharedCtor();
  // @@protoc_insertion_point(constructor:pb.Packet)
}
Packet::Packet(const Packet& from)
  : ::google::protobuf::Message(),
      _internal_metadata_(NULL),
      _cached_size_(0) {
  _internal_metadata_.MergeFrom(from._internal_metadata_);
  type_.UnsafeSetDefault(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
  if (from.type().size() > 0) {
    type_.AssignWithDefault(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), from.type_);
  }
  payload_.UnsafeSetDefault(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
  if (from.payload().size() > 0) {
    payload_.AssignWithDefault(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), from.payload_);
  }
  if (from.has_peer()) {
    peer_ = new ::pb::Peer(*from.peer_);
  } else {
    peer_ = NULL;
  }
  // @@protoc_insertion_point(copy_constructor:pb.Packet)
}

void Packet::SharedCtor() {
  type_.UnsafeSetDefault(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
  payload_.UnsafeSetDefault(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
  peer_ = NULL;
  _cached_size_ = 0;
}

Packet::~Packet() {
  // @@protoc_insertion_point(destructor:pb.Packet)
  SharedDtor();
}

void Packet::SharedDtor() {
  type_.DestroyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
  payload_.DestroyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
  if (this != internal_default_instance()) {
    delete peer_;
  }
}

void Packet::SetCachedSize(int size) const {
  GOOGLE_SAFE_CONCURRENT_WRITES_BEGIN();
  _cached_size_ = size;
  GOOGLE_SAFE_CONCURRENT_WRITES_END();
}
const ::google::protobuf::Descriptor* Packet::descriptor() {
  protobuf_proto_2fdbus_2eproto::protobuf_AssignDescriptorsOnce();
  return protobuf_proto_2fdbus_2eproto::file_level_metadata[1].descriptor;
}

const Packet& Packet::default_instance() {
  protobuf_proto_2fdbus_2eproto::InitDefaults();
  return *internal_default_instance();
}

Packet* Packet::New(::google::protobuf::Arena* arena) const {
  Packet* n = new Packet;
  if (arena != NULL) {
    arena->Own(n);
  }
  return n;
}

void Packet::Clear() {
// @@protoc_insertion_point(message_clear_start:pb.Packet)
  type_.ClearToEmptyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
  payload_.ClearToEmptyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
  if (GetArenaNoVirtual() == NULL && peer_ != NULL) {
    delete peer_;
  }
  peer_ = NULL;
}

bool Packet::MergePartialFromCodedStream(
    ::google::protobuf::io::CodedInputStream* input) {
#define DO_(EXPRESSION) if (!GOOGLE_PREDICT_TRUE(EXPRESSION)) goto failure
  ::google::protobuf::uint32 tag;
  // @@protoc_insertion_point(parse_start:pb.Packet)
  for (;;) {
    ::std::pair< ::google::protobuf::uint32, bool> p = input->ReadTagWithCutoffNoLastTag(127u);
    tag = p.first;
    if (!p.second) goto handle_unusual;
    switch (::google::protobuf::internal::WireFormatLite::GetTagFieldNumber(tag)) {
      // .pb.Peer peer = 1;
      case 1: {
        if (tag == 10u) {
          DO_(::google::protobuf::internal::WireFormatLite::ReadMessageNoVirtual(
               input, mutable_peer()));
        } else {
          goto handle_unusual;
        }
        break;
      }

      // string type = 8;
      case 8: {
        if (tag == 66u) {
          DO_(::google::protobuf::internal::WireFormatLite::ReadString(
                input, this->mutable_type()));
          DO_(::google::protobuf::internal::WireFormatLite::VerifyUtf8String(
            this->type().data(), this->type().length(),
            ::google::protobuf::internal::WireFormatLite::PARSE,
            "pb.Packet.type"));
        } else {
          goto handle_unusual;
        }
        break;
      }

      // bytes payload = 9;
      case 9: {
        if (tag == 74u) {
          DO_(::google::protobuf::internal::WireFormatLite::ReadBytes(
                input, this->mutable_payload()));
        } else {
          goto handle_unusual;
        }
        break;
      }

      default: {
      handle_unusual:
        if (tag == 0 ||
            ::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==
            ::google::protobuf::internal::WireFormatLite::WIRETYPE_END_GROUP) {
          goto success;
        }
        DO_(::google::protobuf::internal::WireFormatLite::SkipField(input, tag));
        break;
      }
    }
  }
success:
  // @@protoc_insertion_point(parse_success:pb.Packet)
  return true;
failure:
  // @@protoc_insertion_point(parse_failure:pb.Packet)
  return false;
#undef DO_
}

void Packet::SerializeWithCachedSizes(
    ::google::protobuf::io::CodedOutputStream* output) const {
  // @@protoc_insertion_point(serialize_start:pb.Packet)
  // .pb.Peer peer = 1;
  if (this->has_peer()) {
    ::google::protobuf::internal::WireFormatLite::WriteMessageMaybeToArray(
      1, *this->peer_, output);
  }

  // string type = 8;
  if (this->type().size() > 0) {
    ::google::protobuf::internal::WireFormatLite::VerifyUtf8String(
      this->type().data(), this->type().length(),
      ::google::protobuf::internal::WireFormatLite::SERIALIZE,
      "pb.Packet.type");
    ::google::protobuf::internal::WireFormatLite::WriteStringMaybeAliased(
      8, this->type(), output);
  }

  // bytes payload = 9;
  if (this->payload().size() > 0) {
    ::google::protobuf::internal::WireFormatLite::WriteBytesMaybeAliased(
      9, this->payload(), output);
  }

  // @@protoc_insertion_point(serialize_end:pb.Packet)
}

::google::protobuf::uint8* Packet::InternalSerializeWithCachedSizesToArray(
    bool deterministic, ::google::protobuf::uint8* target) const {
  (void)deterministic;  // Unused
  // @@protoc_insertion_point(serialize_to_array_start:pb.Packet)
  // .pb.Peer peer = 1;
  if (this->has_peer()) {
    target = ::google::protobuf::internal::WireFormatLite::
      InternalWriteMessageNoVirtualToArray(
        1, *this->peer_, false, target);
  }

  // string type = 8;
  if (this->type().size() > 0) {
    ::google::protobuf::internal::WireFormatLite::VerifyUtf8String(
      this->type().data(), this->type().length(),
      ::google::protobuf::internal::WireFormatLite::SERIALIZE,
      "pb.Packet.type");
    target =
      ::google::protobuf::internal::WireFormatLite::WriteStringToArray(
        8, this->type(), target);
  }

  // bytes payload = 9;
  if (this->payload().size() > 0) {
    target =
      ::google::protobuf::internal::WireFormatLite::WriteBytesToArray(
        9, this->payload(), target);
  }

  // @@protoc_insertion_point(serialize_to_array_end:pb.Packet)
  return target;
}

size_t Packet::ByteSizeLong() const {
// @@protoc_insertion_point(message_byte_size_start:pb.Packet)
  size_t total_size = 0;

  // string type = 8;
  if (this->type().size() > 0) {
    total_size += 1 +
      ::google::protobuf::internal::WireFormatLite::StringSize(
        this->type());
  }

  // bytes payload = 9;
  if (this->payload().size() > 0) {
    total_size += 1 +
      ::google::protobuf::internal::WireFormatLite::BytesSize(
        this->payload());
  }

  // .pb.Peer peer = 1;
  if (this->has_peer()) {
    total_size += 1 +
      ::google::protobuf::internal::WireFormatLite::MessageSizeNoVirtual(
        *this->peer_);
  }

  int cached_size = ::google::protobuf::internal::ToCachedSize(total_size);
  GOOGLE_SAFE_CONCURRENT_WRITES_BEGIN();
  _cached_size_ = cached_size;
  GOOGLE_SAFE_CONCURRENT_WRITES_END();
  return total_size;
}

void Packet::MergeFrom(const ::google::protobuf::Message& from) {
// @@protoc_insertion_point(generalized_merge_from_start:pb.Packet)
  GOOGLE_DCHECK_NE(&from, this);
  const Packet* source =
      ::google::protobuf::internal::DynamicCastToGenerated<const Packet>(
          &from);
  if (source == NULL) {
  // @@protoc_insertion_point(generalized_merge_from_cast_fail:pb.Packet)
    ::google::protobuf::internal::ReflectionOps::Merge(from, this);
  } else {
  // @@protoc_insertion_point(generalized_merge_from_cast_success:pb.Packet)
    MergeFrom(*source);
  }
}

void Packet::MergeFrom(const Packet& from) {
// @@protoc_insertion_point(class_specific_merge_from_start:pb.Packet)
  GOOGLE_DCHECK_NE(&from, this);
  _internal_metadata_.MergeFrom(from._internal_metadata_);
  if (from.type().size() > 0) {

    type_.AssignWithDefault(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), from.type_);
  }
  if (from.payload().size() > 0) {

    payload_.AssignWithDefault(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), from.payload_);
  }
  if (from.has_peer()) {
    mutable_peer()->::pb::Peer::MergeFrom(from.peer());
  }
}

void Packet::CopyFrom(const ::google::protobuf::Message& from) {
// @@protoc_insertion_point(generalized_copy_from_start:pb.Packet)
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

void Packet::CopyFrom(const Packet& from) {
// @@protoc_insertion_point(class_specific_copy_from_start:pb.Packet)
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

bool Packet::IsInitialized() const {
  return true;
}

void Packet::Swap(Packet* other) {
  if (other == this) return;
  InternalSwap(other);
}
void Packet::InternalSwap(Packet* other) {
  type_.Swap(&other->type_);
  payload_.Swap(&other->payload_);
  std::swap(peer_, other->peer_);
  std::swap(_cached_size_, other->_cached_size_);
}

::google::protobuf::Metadata Packet::GetMetadata() const {
  protobuf_proto_2fdbus_2eproto::protobuf_AssignDescriptorsOnce();
  return protobuf_proto_2fdbus_2eproto::file_level_metadata[1];
}

#if PROTOBUF_INLINE_NOT_IN_HEADERS
// Packet

// .pb.Peer peer = 1;
bool Packet::has_peer() const {
  return this != internal_default_instance() && peer_ != NULL;
}
void Packet::clear_peer() {
  if (GetArenaNoVirtual() == NULL && peer_ != NULL) delete peer_;
  peer_ = NULL;
}
const ::pb::Peer& Packet::peer() const {
  // @@protoc_insertion_point(field_get:pb.Packet.peer)
  return peer_ != NULL ? *peer_
                         : *::pb::Peer::internal_default_instance();
}
::pb::Peer* Packet::mutable_peer() {
  
  if (peer_ == NULL) {
    peer_ = new ::pb::Peer;
  }
  // @@protoc_insertion_point(field_mutable:pb.Packet.peer)
  return peer_;
}
::pb::Peer* Packet::release_peer() {
  // @@protoc_insertion_point(field_release:pb.Packet.peer)
  
  ::pb::Peer* temp = peer_;
  peer_ = NULL;
  return temp;
}
void Packet::set_allocated_peer(::pb::Peer* peer) {
  delete peer_;
  peer_ = peer;
  if (peer) {
    
  } else {
    
  }
  // @@protoc_insertion_point(field_set_allocated:pb.Packet.peer)
}

// string type = 8;
void Packet::clear_type() {
  type_.ClearToEmptyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
const ::std::string& Packet::type() const {
  // @@protoc_insertion_point(field_get:pb.Packet.type)
  return type_.GetNoArena();
}
void Packet::set_type(const ::std::string& value) {
  
  type_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), value);
  // @@protoc_insertion_point(field_set:pb.Packet.type)
}
#if LANG_CXX11
void Packet::set_type(::std::string&& value) {
  
  type_.SetNoArena(
    &::google::protobuf::internal::GetEmptyStringAlreadyInited(), std::move(value));
  // @@protoc_insertion_point(field_set_rvalue:pb.Packet.type)
}
#endif
void Packet::set_type(const char* value) {
  
  type_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::string(value));
  // @@protoc_insertion_point(field_set_char:pb.Packet.type)
}
void Packet::set_type(const char* value, size_t size) {
  
  type_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(),
      ::std::string(reinterpret_cast<const char*>(value), size));
  // @@protoc_insertion_point(field_set_pointer:pb.Packet.type)
}
::std::string* Packet::mutable_type() {
  
  // @@protoc_insertion_point(field_mutable:pb.Packet.type)
  return type_.MutableNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
::std::string* Packet::release_type() {
  // @@protoc_insertion_point(field_release:pb.Packet.type)
  
  return type_.ReleaseNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
void Packet::set_allocated_type(::std::string* type) {
  if (type != NULL) {
    
  } else {
    
  }
  type_.SetAllocatedNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), type);
  // @@protoc_insertion_point(field_set_allocated:pb.Packet.type)
}

// bytes payload = 9;
void Packet::clear_payload() {
  payload_.ClearToEmptyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
const ::std::string& Packet::payload() const {
  // @@protoc_insertion_point(field_get:pb.Packet.payload)
  return payload_.GetNoArena();
}
void Packet::set_payload(const ::std::string& value) {
  
  payload_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), value);
  // @@protoc_insertion_point(field_set:pb.Packet.payload)
}
#if LANG_CXX11
void Packet::set_payload(::std::string&& value) {
  
  payload_.SetNoArena(
    &::google::protobuf::internal::GetEmptyStringAlreadyInited(), std::move(value));
  // @@protoc_insertion_point(field_set_rvalue:pb.Packet.payload)
}
#endif
void Packet::set_payload(const char* value) {
  
  payload_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::string(value));
  // @@protoc_insertion_point(field_set_char:pb.Packet.payload)
}
void Packet::set_payload(const void* value, size_t size) {
  
  payload_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(),
      ::std::string(reinterpret_cast<const char*>(value), size));
  // @@protoc_insertion_point(field_set_pointer:pb.Packet.payload)
}
::std::string* Packet::mutable_payload() {
  
  // @@protoc_insertion_point(field_mutable:pb.Packet.payload)
  return payload_.MutableNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
::std::string* Packet::release_payload() {
  // @@protoc_insertion_point(field_release:pb.Packet.payload)
  
  return payload_.ReleaseNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
void Packet::set_allocated_payload(::std::string* payload) {
  if (payload != NULL) {
    
  } else {
    
  }
  payload_.SetAllocatedNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), payload);
  // @@protoc_insertion_point(field_set_allocated:pb.Packet.payload)
}

#endif  // PROTOBUF_INLINE_NOT_IN_HEADERS

// @@protoc_insertion_point(namespace_scope)

}  // namespace pb

// @@protoc_insertion_point(global_scope)
