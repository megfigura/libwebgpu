#pragma once

#include <vector>
#include <string>
#include <tl/expected.hpp>

#include "Resource.h"

class RawResource : public Resource
{
public:
    explicit RawResource(const std::filesystem::path& resourceDir, const std::filesystem::path& filename);
    [[nodiscard]] bool isLoadable(std::string& error) const override;
    [[nodiscard]] tl::expected<std::shared_ptr<std::vector<char>>, std::string> getBytes() const;

private:
    tl::expected<std::shared_ptr<std::vector<char>>, std::string> m_data;

    static tl::expected<std::shared_ptr<std::vector<char>>, std::string> loadResource(const std::filesystem::path& filename);
};
