#pragma once

#include "common.hpp"

namespace gabenet::message {
struct Value {
	SteamNetworkingMessage_t *native = nullptr;
};

Napi::Value create(Napi::Env env, SteamNetworkingMessage_t *native);
Value *require(Napi::Env env, const Napi::Value &value);
} // namespace gabenet::message
