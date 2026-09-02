#pragma once

#include <addon-tools.hpp>

#include <steam/isteamnetworkingmessages.h>
#include <steam/isteamnetworkingutils.h>
#include <steam/steamnetworkingsockets.h>

#include <string>

namespace gabenet {
bool initialized();
ISteamNetworkingSockets *networkingSockets(Napi::Env env);
ISteamNetworkingMessages *networkingMessages(Napi::Env env);
ISteamNetworkingUtils *networkingUtils(Napi::Env env);
bool requirePort(Napi::Env env, const Napi::CallbackInfo &info, uint32 index, uint16 *port);
bool requireAddress(
    Napi::Env env, const Napi::CallbackInfo &info, uint32 index, uint16 port, SteamNetworkingIPAddr *address
);
bool requireIdentity(
    Napi::Env env, const Napi::CallbackInfo &info, uint32 index, SteamNetworkingIdentity *identity
);
std::string identityToString(const SteamNetworkingIdentity &identity);
Napi::Array copyMessages(Napi::Env env, SteamNetworkingMessage_t **messages, int count);
} // namespace gabenet
