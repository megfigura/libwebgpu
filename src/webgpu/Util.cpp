#include "Util.h"
#include <thread>
#include <chrono>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

namespace webgpu
{
    void Util::sleep(int millis)
    {
#ifdef __EMSCRIPTEN__
        emscripten_sleep(millis);
#else
        std::this_thread::sleep_for(std::chrono::milliseconds(millis));
#endif
    }
}
