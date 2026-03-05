#include "BasePass.h"

namespace webgpu
{
    BasePass::BasePass(const std::string_view name) : m_name{name}
    {
    }

    std::string_view BasePass::getName()
    {
        return m_name;
    }
}
