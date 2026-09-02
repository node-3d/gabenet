#pragma once

#include "common.hpp"

namespace gabenet::events {
void registerCallbacks();
void clear();
void setDebugOutputLevel(ESteamNetworkingSocketsDebugOutputType level);
JS_METHOD(pollEvents);
} // namespace gabenet::events
