#pragma once

namespace webgpu
{
    class Util
    {
    public:
        static void sleep(int millis);

        template <typename T>
        static T nextPow2Multiple(T val, const int multiple)
        {
            return (val + (multiple - 1)) & -multiple;
        }
    };
}
