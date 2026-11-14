#ifndef SDCS_CLUSTER_H_
#define SDCS_CLUSTER_H_

#include <chrono>
#include <memory>
#include <optional>
#include "sdcs/storage.h"
#include "sdcs/types.h"

namespace sdcs {

struct ClusterOptions {
    NodeAddress self;
    std::optional<NodeAddress> seed;
    std::chrono::milliseconds rpc_timeout{1000};
    std::chrono::milliseconds heartbeat_interval{1000};
    std::uint32_t heartbeat_failure_threshold = 3;
};

class ClusterService {
public:
    virtual ~ClusterService() = default;
    virtual Status Start(const ClusterOptions& options, std::shared_ptr<LocalStore> store) = 0;
    virtual GetResult Get(const std::string& key, const RequestContext& context) = 0;
    virtual PutResult Put(const std::string& key, const nlohmann::json& value, const RequestContext& context) = 0;
    virtual DeleteResult Delete(const std::string& key, const RequestContext& context) = 0;
    virtual HealthSnapshot Health() const = 0;
    virtual Status Stop(const RequestContext& context) = 0;
};

std::unique_ptr<ClusterService> MakeClusterService();

}  // namespace sdcs

#endif  // SDCS_CLUSTER_H_

