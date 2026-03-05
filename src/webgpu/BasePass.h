#pragma once
#include <string_view>
#include <string>

namespace webgpu
{
    class BasePass
    {
    public:
        explicit BasePass(std::string_view name);
        virtual ~BasePass() = default;

        std::string_view getName();

    private:
        std::string m_name;
    };
}