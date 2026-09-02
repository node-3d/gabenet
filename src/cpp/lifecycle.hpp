#pragma once

#include "common.hpp"

namespace gabenet {
bool initialized();
void shutdown();
JS_METHOD(init);
JS_METHOD(shutdownBinding);
JS_METHOD(isInitialized);
JS_METHOD(runCallbacks);
} // namespace gabenet
