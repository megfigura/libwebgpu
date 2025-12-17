#pragma once

#include <vector>
#include <string>
#include <tl/expected.hpp>

#include "Resource.h"

namespace resource
{
    class RawResource : public Resource
    {
    public:
        RawResource(const std::filesystem::path& resourceDir, const std::filesystem::path& filename);
        RawResource(const std::filesystem::path& resourceDir, const std::filesystem::path& filename, const std::vector<char>& data);
        [[nodiscard]] const std::vector<char>& getBytes() const;

    private:
        std::vector<char> m_data;

        static tl::expected<std::vector<char>, std::string> loadResource(const std::filesystem::path& filename);
    };
}
