#pragma once
#include <filesystem>
#include <string>

class Resource
{
public:
    explicit Resource(const std::filesystem::path& resourceDir, const std::filesystem::path& path);
    Resource(const Resource& other);
    virtual ~Resource();

    [[nodiscard]] virtual bool isLoadable(std::string& error) const;
    [[nodiscard]] virtual std::string getName() const;
    [[nodiscard]] virtual std::filesystem::path getPath() const;
    [[nodiscard]] std::filesystem::path getResourceDir() const;

private:
    std::filesystem::path m_resourceDir;
    std::filesystem::path m_filename;
};
