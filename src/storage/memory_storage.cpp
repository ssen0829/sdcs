#include "sdcs/storage.h"
#include <mutex>
#include <unordered_map>

namespace sdcs {

class MemoryLocalStore : public LocalStore {
public:
    LocalReadResult ReadLocal(const std::string& key) const override {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = store_.find(key);
        if (it == store_.end()) {
            return {Status{Code::NotFound, "Key not found"}, std::nullopt};
        }
        return {Status{Code::Ok, ""}, it->second};
    }

    ApplyResult ApplyLocal(const std::string& key, const Record& record) override {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = store_.find(key);
        if (it != store_.end()) {
            const auto& current = it->second;
            if (record.version < current.version) {
                return {Status{Code::Ok, ""}, ApplyDisposition::Superseded, current.version};
            }
            if (record.version == current.version) {
                return {Status{Code::Ok, ""}, ApplyDisposition::AlreadyPresent, current.version};
            }
        }
        store_[key] = record;
        return {Status{Code::Ok, ""}, ApplyDisposition::Applied, record.version};
    }

    SnapshotResult SnapshotLocal() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        std::vector<Entry> entries;
        entries.reserve(store_.size());
        for (const auto& [k, v] : store_) {
            entries.push_back({k, v});
        }
        return {Status{Code::Ok, ""}, std::move(entries)};
    }

    EraseResult EraseLocalIfVersion(const std::string& key, const Version& expected) override {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = store_.find(key);
        if (it != store_.end() && it->second.version == expected) {
            store_.erase(it);
            return {Status{Code::Ok, ""}, true};
        }
        return {Status{Code::Ok, ""}, false};
    }

private:
    mutable std::mutex mutex_;
    std::unordered_map<std::string, Record> store_;
};

std::shared_ptr<LocalStore> MakeLocalStore() {
    return std::make_shared<MemoryLocalStore>();
}

}  // namespace sdcs
