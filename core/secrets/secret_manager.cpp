#include "secret_manager.hpp"

#include <cctype>
#include <cstdlib>
#include <chrono>
#include <fstream>
#include <system_error>

namespace sovereign::secrets
{
namespace
{

std::uint64_t NowUnix()
{
    using namespace std::chrono;
    return static_cast<std::uint64_t>(
        duration_cast<seconds>(system_clock::now().time_since_epoch()).count());
}

} // namespace

bool SecretManager::ValidName(const std::string& name)
{
    if (name.empty() || name.size() > 128)
        return false;
    for (unsigned char c : name)
    {
        if (std::isalnum(c) || c == '_' || c == '-' || c == '.')
            continue;
        return false;
    }
    if (name.find("..") != std::string::npos)
        return false;
    return true;
}

SecretError SecretManager::Open(const std::filesystem::path& data_root)
{
    std::lock_guard lock(mutex_);
    std::error_code ec;
    root_ = data_root / "secrets";
    std::filesystem::create_directories(root_ / "items", ec);
    if (ec)
        return SecretError::StorageFailure;
    std::filesystem::permissions(
        root_,
        std::filesystem::perms::owner_all,
        std::filesystem::perm_options::replace,
        ec);
    open_ = true;
    return SecretError::Ok;
}

void SecretManager::Close() noexcept
{
    std::lock_guard lock(mutex_);
    open_ = false;
    root_.clear();
}

void SecretManager::SetAuditHook(AuditHook hook)
{
    std::lock_guard lock(mutex_);
    audit_ = std::move(hook);
}

void SecretManager::Audit(const std::string& action, const std::string& name) const
{
    if (audit_)
        audit_(action, name);
}

std::filesystem::path SecretManager::ItemDir(const std::string& name) const
{
    return root_ / "items" / name;
}

Result<void> SecretManager::Put(const std::string& name, const std::string& value)
{
    std::lock_guard lock(mutex_);
    if (!open_)
        return Result<void>::Err(SecretError::NotOpen);
    if (!ValidName(name))
        return Result<void>::Err(SecretError::InvalidArgument);
    std::error_code ec;
    auto dir = ItemDir(name);
    std::filesystem::create_directories(dir, ec);
    if (ec)
        return Result<void>::Err(SecretError::StorageFailure);
    auto val_path = dir / "value";
    auto meta_path = dir / "meta";
    {
        std::ofstream out(val_path, std::ios::binary | std::ios::trunc);
        if (!out)
            return Result<void>::Err(SecretError::StorageFailure);
        out.write(value.data(), static_cast<std::streamsize>(value.size()));
    }
    std::filesystem::permissions(
        val_path,
        std::filesystem::perms::owner_read | std::filesystem::perms::owner_write,
        std::filesystem::perm_options::replace,
        ec);
    {
        std::ofstream meta(meta_path, std::ios::trunc);
        if (!meta)
            return Result<void>::Err(SecretError::StorageFailure);
        meta << "name=" << name << "\n"
             << "updated_unix=" << NowUnix() << "\n"
             << "value_bytes=" << value.size() << "\n";
    }
    Audit("put", name);
    return Result<void>::OkRes();
}

Result<std::string> SecretManager::Get(const std::string& name)
{
    Result<std::string> r;
    std::lock_guard lock(mutex_);
    if (!open_)
    {
        r.error = SecretError::NotOpen;
        return r;
    }
    if (!ValidName(name))
    {
        r.error = SecretError::InvalidArgument;
        return r;
    }
    auto val_path = ItemDir(name) / "value";
    if (!std::filesystem::exists(val_path))
    {
        r.error = SecretError::NotFound;
        return r;
    }
    std::ifstream in(val_path, std::ios::binary);
    if (!in)
    {
        r.error = SecretError::StorageFailure;
        return r;
    }
    r.value.assign(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());
    r.error = SecretError::Ok;
    Audit("get", name);
    return r;
}

Result<bool> SecretManager::Exists(const std::string& name)
{
    Result<bool> r;
    std::lock_guard lock(mutex_);
    if (!open_)
    {
        r.error = SecretError::NotOpen;
        return r;
    }
    if (!ValidName(name))
    {
        r.error = SecretError::InvalidArgument;
        return r;
    }
    r.value = std::filesystem::exists(ItemDir(name) / "value");
    r.error = SecretError::Ok;
    Audit("exists", name);
    return r;
}

Result<std::vector<SecretMeta>> SecretManager::List() const
{
    Result<std::vector<SecretMeta>> r;
    std::lock_guard lock(mutex_);
    if (!open_)
    {
        r.error = SecretError::NotOpen;
        return r;
    }
    std::error_code ec;
    auto items = root_ / "items";
    if (!std::filesystem::exists(items))
    {
        r.error = SecretError::Ok;
        return r;
    }
    for (const auto& ent : std::filesystem::directory_iterator(items, ec))
    {
        if (!ent.is_directory())
            continue;
        SecretMeta m;
        m.name = ent.path().filename().string();
        auto meta_path = ent.path() / "meta";
        if (std::ifstream meta{meta_path})
        {
            std::string line;
            while (std::getline(meta, line))
            {
                if (line.rfind("updated_unix=", 0) == 0)
                    m.updated_unix = std::strtoull(line.c_str() + 13, nullptr, 10);
                if (line.rfind("value_bytes=", 0) == 0)
                    m.value_bytes = static_cast<std::size_t>(std::strtoull(line.c_str() + 12, nullptr, 10));
            }
        }
        r.value.push_back(std::move(m));
    }
    if (audit_)
        audit_("list", "*");
    r.error = SecretError::Ok;
    return r;
}

Result<void> SecretManager::Delete(const std::string& name)
{
    std::lock_guard lock(mutex_);
    if (!open_)
        return Result<void>::Err(SecretError::NotOpen);
    if (!ValidName(name))
        return Result<void>::Err(SecretError::InvalidArgument);
    std::error_code ec;
    auto dir = ItemDir(name);
    if (!std::filesystem::exists(dir))
        return Result<void>::Err(SecretError::NotFound);
    std::filesystem::remove_all(dir, ec);
    if (ec)
        return Result<void>::Err(SecretError::StorageFailure);
    Audit("delete", name);
    return Result<void>::OkRes();
}

} // namespace sovereign::secrets
