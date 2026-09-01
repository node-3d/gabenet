#include <addon-tools.hpp>

#include <steam/steamnetworkingsockets.h>

namespace {
bool initialized = false;

void shutdown() {
	if (!initialized)
		return;
	GameNetworkingSockets_Kill();
	initialized = false;
}

JS_METHOD(init) {
	NAPI_ENV;
	Napi::Object result = JS_OBJECT;
	if (initialized) {
		result.Set("ok", true);
		result.Set("errorMessage", "");
		RET_VALUE(result);
	}
	SteamNetworkingErrMsg errorMessage = {};
	initialized = GameNetworkingSockets_Init(nullptr, errorMessage);
	result.Set("ok", initialized);
	result.Set("errorMessage", std::string(errorMessage));
	RET_VALUE(result);
}

JS_METHOD(shutdownBinding) {
	NAPI_ENV;
	shutdown();
	RET_UNDEFINED;
}

JS_METHOD(isInitialized) {
	NAPI_ENV;
	RET_BOOL(initialized);
}

JS_METHOD(runCallbacks) {
	NAPI_ENV;
	if (initialized)
		SteamNetworkingSockets()->RunCallbacks();
	RET_UNDEFINED;
}

Napi::Object initModule(Napi::Env env, Napi::Object exports) {
	env.AddCleanupHook(shutdown);
	exports.Set("init", Napi::Function::New(env, init));
	exports.Set("shutdown", Napi::Function::New(env, shutdownBinding));
	exports.Set("isInitialized", Napi::Function::New(env, isInitialized));
	exports.Set("runCallbacks", Napi::Function::New(env, runCallbacks));
	return exports;
}
} // namespace

NODE_API_MODULE(gabenet, initModule)
