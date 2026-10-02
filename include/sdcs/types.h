#ifndef SDCS_TYPES_H_
#ifndef SDCS_TYPES_H_
#define SDCS_TYPES_H_

#include <chrono>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

namespace sdcs {

enum class Code {
    Ok, NotFound, InvalidArgument, Conflict, Unavailable,
    DeadlineExceeded, ResourceExhausted, Internal
};

struct Status {
    Code code = Code::Internal;
    std::string message;
};

struct Version {
    std::uint64_t counter = 0;
    std::string writer_incarnation;
};

struct Record {
    nlohmann::json value = nullptr;
    Version version;
    bool tombstone = false;
    std::string mutation_id;
};

struct Entry {
    std::string key;
    Record record;
};

struct RequestContext {
    std::string request_id;
    std::chrono::steady_clock::time_point deadline;
};

struct GetResult {
    Status status;
    std::optional<nlohmann::json> value;
};

struct PutResult {
    Status status;
};

struct DeleteResult {
    Status status;
    std::uint32_t deleted = 0;
};

struct LocalReadResult {
    Status status;
    std::optional<Record> record;
};

enum class ApplyDisposition {
    Applied, AlreadyPresent, Superseded, Rejected
};

struct ApplyResult {
    Status status;
    ApplyDisposition disposition = ApplyDisposition::Rejected;
    std::optional<Version> current_version;
};

struct SnapshotResult {
    Status status;
    std::vector<Entry> entries;
};

struct EraseResult {
    Status status;
    bool erased = false;
};

struct NodeAddress {
    std::string ip;
    std::uint16_t http_port = 0;
};

enum class NodeState {
    Starting, Joining, Active, Recovering, Reconfiguring, Leaving, Stopped
};

struct HealthSnapshot {
    NodeState state = NodeState::Starting;
    bool accepting_requests = false;
    bool degraded = false;
    std::uint64_t membership_epoch = 0;
    std::string detail;
};

}  // namespace sdcs

#endif  // SDCS_TYPES_H_
