#pragma once
#include <filesystem>
#include <optional>
#include <string>

namespace resource
{
    class Resource
    {
    public:
        explicit Resource(std::filesystem::path resourceDir, std::filesystem::path path);
        virtual ~Resource();

        [[nodiscard]] virtual bool isOk(std::string& error) const;
        [[nodiscard]] virtual std::string getName() const;
        [[nodiscard]] virtual std::filesystem::path getPath() const;
        [[nodiscard]] std::filesystem::path getResourceDir() const;

    protected:
        void setError(const std::string& errorMessage);

    private:
        std::filesystem::path m_resourceDir;
        std::filesystem::path m_filename;
        std::optional<std::string> m_error;
    };
}
