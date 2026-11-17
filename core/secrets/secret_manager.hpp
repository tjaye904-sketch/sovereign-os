#pragma once

#include "secret_types.hpp"

#include <filesystem>
#include <functional>
#include <mutex>
#include <string>

namespace sovereign::secrets
{

// File-backed secret store under data_root/secrets.
// Values never written to audit; only name + action.
class SecretManager
{
public:
    using AuditHook = std::function<void(const std::string& action, const std::string& name)>;

    SecretError Open(const std::filesystem::path& data_root);
    void Close() noexcept;
    void SetAuditHook(AuditHook hook);

    Result<void> Put(const std::string& name, const std::string& value);
    Result<std::string> Get(const std::string& name);
    Result<bool> Exists(const std::string& name);
    Result<std::vector<SecretMeta>> List() const;
    Result<void> Delete(const std::string& name);

    bool IsOpen() const noexcept { return open_; }

private:
    static bool ValidName(const std::string& name);
    std::filesystem::path ItemDir(const std::string& name) const;
    void Audit(const std::string& action, const std::string& name) const;

    mutable std::mutex mutex_;
    bool open_{false};
    std::filesystem::path root_;
    AuditHook audit_;
};

} // namespace sovereign::secrets
