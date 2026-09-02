#include "message.hpp"

namespace gabenet::message {
namespace {
void finalize(Napi::Env, Value *value) {
	if (value->native != nullptr)
		value->native->Release();
	delete value;
}
} // namespace

Napi::Value create(Napi::Env env, SteamNetworkingMessage_t *native) {
	return Napi::External<Value>::New(env, new Value{ native }, finalize);
}

Value *require(Napi::Env env, const Napi::Value &value) {
	if (!value.IsExternal()) {
		JS_THROW("message must be a message returned by utils.allocateMessage().");
		return nullptr;
	}
	Value *result = value.As<Napi::External<Value>>().Data();
	if (result->native == nullptr) {
		JS_THROW("message has already been sent or released.");
		return nullptr;
	}
	return result;
}
} // namespace gabenet::message
