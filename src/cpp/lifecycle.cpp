#include "lifecycle.hpp"

#include "events.hpp"

namespace gabenet {
namespace {
bool libraryInitialized = false;
}

bool initialized() {
	return libraryInitialized;
}

void shutdown() {
	if (!libraryInitialized)
		return;
	GameNetworkingSockets_Kill();
	libraryInitialized = false;
	events::clear();
}

JS_METHOD(init) {
	NAPI_ENV;
	Napi::Object result = JS_OBJECT;
	if (libraryInitialized) {
		result.Set("ok", true);
		result.Set("errorMessage", "");
		RET_VALUE(result);
	}
	SteamNetworkingErrMsg errorMessage = {};
	libraryInitialized = GameNetworkingSockets_Init(nullptr, errorMessage);
	if (libraryInitialized)
		events::registerCallbacks();
	result.Set("ok", libraryInitialized);
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
	RET_BOOL(libraryInitialized);
}

JS_METHOD(runCallbacks) {
	NAPI_ENV;
	if (libraryInitialized)
		SteamNetworkingSockets()->RunCallbacks();
	RET_UNDEFINED;
}
} // namespace gabenet
