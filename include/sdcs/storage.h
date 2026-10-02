#ifndef SDCS_STORAGE_H_
#define SDCS_STORAGE_H_

#include <memory>
#include "sdcs/types.h"

namespace sdcs {

class LocalStore {
public:
    virtual ~LocalStore() = default;
    virtual LocalReadResult ReadLocal(const std::string& key) const = 0;
    virtual ApplyResult ApplyLocal(const std::string& key, const Record& record) = 0;
    virtual SnapshotResult SnapshotLocal() const = 0;
    virtual EraseResult EraseLocalIfVersion(const std::string& key, const Version& expected) = 0;
};

std::shared_ptr<LocalStore> MakeLocalStore();

}  // namespace sdcs

#endif  // SDCS_STORAGE_H_
