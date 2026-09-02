#include "events.hpp"
#include "lifecycle.hpp"
#include "messages.hpp"
#include "sockets.hpp"
#include "utils.hpp"

Napi::Object initModule(Napi::Env env, Napi::Object exports) {
	env.AddCleanupHook(gabenet::shutdown);
	exports.Set("init", Napi::Function::New(env, gabenet::init));
	exports.Set("shutdown", Napi::Function::New(env, gabenet::shutdownBinding));
	exports.Set("isInitialized", Napi::Function::New(env, gabenet::isInitialized));
	exports.Set("runCallbacks", Napi::Function::New(env, gabenet::runCallbacks));
	exports.Set("pollEvents", Napi::Function::New(env, gabenet::events::pollEvents));
	exports.Set("sockets", gabenet::sockets::createNamespace(env));
	exports.Set("messages", gabenet::messages::createNamespace(env));
	exports.Set("utils", gabenet::utils::createNamespace(env));
	return exports;
}

NODE_API_MODULE(gabenet, initModule)
