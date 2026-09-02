#include "messages.hpp"

#include <vector>

namespace gabenet::messages {
namespace {
JS_METHOD(sendMessageToUser) {
	NAPI_ENV;
	SteamNetworkingIdentity identity;
	if (!requireIdentity(env, info, 0, &identity))
		RET_UNDEFINED;
	REQ_BUF_ARG(1, data);
	if (data.Length() > k_cbMaxSteamNetworkingSocketsMessageSizeSend) {
		JS_THROW("data exceeds the maximum GNS message size.");
		RET_UNDEFINED;
	}
	USE_INT32_ARG(2, sendFlags, k_nSteamNetworkingSend_Reliable);
	USE_INT32_ARG(3, remoteChannel, 0);
	ISteamNetworkingMessages *value = networkingMessages(env);
	if (value == nullptr)
		RET_UNDEFINED;
	RET_NUM(
	    static_cast<int32>(value->SendMessageToUser(
	        identity, data.Data(), static_cast<uint32>(data.Length()), sendFlags, remoteChannel
	    ))
	);
}

JS_METHOD(receiveMessagesOnChannel) {
	NAPI_ENV;
	REQ_INT32_ARG(0, localChannel);
	USE_INT32_ARG(1, maximumMessages, 32);
	if (maximumMessages < 1 || maximumMessages > 256) {
		JS_THROW("maximumMessages must be between 1 and 256.");
		RET_UNDEFINED;
	}
	ISteamNetworkingMessages *value = networkingMessages(env);
	if (value == nullptr)
		RET_UNDEFINED;
	std::vector<SteamNetworkingMessage_t *> received(maximumMessages);
	int count = value->ReceiveMessagesOnChannel(localChannel, received.data(), maximumMessages);
	RET_VALUE(copyMessages(env, received.data(), count));
}

JS_METHOD(acceptSessionWithUser) {
	NAPI_ENV;
	SteamNetworkingIdentity identity;
	if (!requireIdentity(env, info, 0, &identity))
		RET_UNDEFINED;
	ISteamNetworkingMessages *value = networkingMessages(env);
	if (value == nullptr)
		RET_UNDEFINED;
	RET_BOOL(value->AcceptSessionWithUser(identity));
}

JS_METHOD(closeSessionWithUser) {
	NAPI_ENV;
	SteamNetworkingIdentity identity;
	if (!requireIdentity(env, info, 0, &identity))
		RET_UNDEFINED;
	ISteamNetworkingMessages *value = networkingMessages(env);
	if (value == nullptr)
		RET_UNDEFINED;
	RET_BOOL(value->CloseSessionWithUser(identity));
}

JS_METHOD(closeChannelWithUser) {
	NAPI_ENV;
	SteamNetworkingIdentity identity;
	if (!requireIdentity(env, info, 0, &identity))
		RET_UNDEFINED;
	REQ_INT32_ARG(1, localChannel);
	ISteamNetworkingMessages *value = networkingMessages(env);
	if (value == nullptr)
		RET_UNDEFINED;
	RET_BOOL(value->CloseChannelWithUser(identity, localChannel));
}

JS_METHOD(getSessionConnectionInfo) {
	NAPI_ENV;
	SteamNetworkingIdentity identity;
	if (!requireIdentity(env, info, 0, &identity))
		RET_UNDEFINED;
	ISteamNetworkingMessages *value = networkingMessages(env);
	if (value == nullptr)
		RET_UNDEFINED;
	SteamNetConnectionInfo_t connectionInfo = {};
	SteamNetConnectionRealTimeStatus_t realTimeStatus = {};
	ESteamNetworkingConnectionState state =
	    value->GetSessionConnectionInfo(identity, &connectionInfo, &realTimeStatus);
	Napi::Object result = JS_OBJECT;
	result.Set("state", static_cast<int32>(state));
	result.Set("identityRemote", identityToString(identity));
	result.Set("endReason", connectionInfo.m_eEndReason);
	result.Set("endDebug", connectionInfo.m_szEndDebug);
	result.Set("pendingUnreliable", realTimeStatus.m_cbPendingUnreliable);
	result.Set("pendingReliable", realTimeStatus.m_cbPendingReliable);
	result.Set("sentUnackedReliable", realTimeStatus.m_cbSentUnackedReliable);
	result.Set("queueTime", std::to_string(realTimeStatus.m_usecQueueTime));
	RET_VALUE(result);
}
} // namespace

Napi::Object createNamespace(Napi::Env env) {
	Napi::Object value = JS_OBJECT;
	value.Set("sendMessageToUser", Napi::Function::New(env, sendMessageToUser));
	value.Set("receiveMessagesOnChannel", Napi::Function::New(env, receiveMessagesOnChannel));
	value.Set("acceptSessionWithUser", Napi::Function::New(env, acceptSessionWithUser));
	value.Set("closeSessionWithUser", Napi::Function::New(env, closeSessionWithUser));
	value.Set("closeChannelWithUser", Napi::Function::New(env, closeChannelWithUser));
	value.Set("getSessionConnectionInfo", Napi::Function::New(env, getSessionConnectionInfo));
	return value;
}
} // namespace gabenet::messages
