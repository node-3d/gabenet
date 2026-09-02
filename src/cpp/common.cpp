#include "common.hpp"

#include "lifecycle.hpp"

#include <cmath>
#include <vector>

namespace gabenet {
ISteamNetworkingSockets *networkingSockets(Napi::Env env) {
	if (!initialized()) {
		JS_THROW("GameNetworkingSockets is not initialized. Call init() first.");
		return nullptr;
	}
	return SteamNetworkingSockets_Lib();
}

ISteamNetworkingMessages *networkingMessages(Napi::Env env) {
	if (!initialized()) {
		JS_THROW("GameNetworkingSockets is not initialized. Call init() first.");
		return nullptr;
	}
	return SteamNetworkingMessages_Lib();
}

ISteamNetworkingUtils *networkingUtils(Napi::Env env) {
	if (!initialized()) {
		JS_THROW("GameNetworkingSockets is not initialized. Call init() first.");
		return nullptr;
	}
	return SteamNetworkingUtils_Lib();
}

bool requirePort(Napi::Env env, const Napi::CallbackInfo &info, uint32 index, uint16 *port) {
	if (info.Length() <= index || !info[index].IsNumber()) {
		JS_THROW("port must be an integer between 0 and 65535.");
		return false;
	}
	double value = info[index].As<Napi::Number>().DoubleValue();
	if (value < 0 || value > 65535 || std::trunc(value) != value) {
		JS_THROW("port must be an integer between 0 and 65535.");
		return false;
	}
	*port = static_cast<uint16>(value);
	return true;
}

bool requireAddress(
    Napi::Env env, const Napi::CallbackInfo &info, uint32 index, uint16 port, SteamNetworkingIPAddr *address
) {
	if (info.Length() <= index || !info[index].IsString()) {
		JS_THROW("host must be an IPv4 or IPv6 address string.");
		return false;
	}
	address->Clear();
	if (!address->ParseString(info[index].As<Napi::String>().Utf8Value().c_str())) {
		JS_THROW("host must be an IPv4 or IPv6 address string.");
		return false;
	}
	address->m_port = port;
	return true;
}

bool requireIdentity(
    Napi::Env env, const Napi::CallbackInfo &info, uint32 index, SteamNetworkingIdentity *identity
) {
	if (info.Length() <= index || !info[index].IsString()) {
		JS_THROW("identityRemote must be a valid GameNetworkingSockets identity string.");
		return false;
	}
	identity->Clear();
	if (!identity->ParseString(info[index].As<Napi::String>().Utf8Value().c_str()) || identity->IsInvalid()) {
		JS_THROW("identityRemote must be a valid GameNetworkingSockets identity string.");
		return false;
	}
	return true;
}

std::string identityToString(const SteamNetworkingIdentity &identity) {
	char text[SteamNetworkingIdentity::k_cchMaxString] = {};
	identity.ToString(text, sizeof(text));
	return text;
}

Napi::Array copyMessages(Napi::Env env, SteamNetworkingMessage_t **messages, int count) {
	Napi::Array result = Napi::Array::New(env, count);
	for (int index = 0; index < count; ++index) {
		SteamNetworkingMessage_t *message = messages[index];
		Napi::Object value = JS_OBJECT;
		value.Set(
		    "data", Napi::Buffer<char>::Copy(env, static_cast<char *>(message->m_pData), message->m_cbSize)
		);
		value.Set("connection", message->m_conn);
		value.Set("identityRemote", identityToString(message->m_identityPeer));
		value.Set("channel", message->m_nChannel);
		value.Set("messageNumber", std::to_string(message->m_nMessageNumber));
		value.Set("receivedAt", std::to_string(message->m_usecTimeReceived));
		message->Release();
		result.Set(index, value);
	}
	return result;
}
} // namespace gabenet
