#pragma once
#include <filesystem>
#include <string>

class Resource
{
public:
    explicit Resource(std::filesystem::path  filename);
    Resource(const Resource& other);
    virtual ~Resource();

    [[nodiscard]] virtual bool isValid() const;
    [[nodiscard]] virtual std::string getError() const;
    [[nodiscard]] virtual std::string getName() const;
    [[nodiscard]] virtual std::filesystem::path getFilename() const;

private:
    std::filesystem::path m_filename;
};
