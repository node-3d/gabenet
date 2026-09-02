#include "sockets.hpp"

#include "message.hpp"

#include <vector>

namespace gabenet::sockets {
namespace {
JS_METHOD(createListenSocketIP) {
	NAPI_ENV;
	uint16 port = 0;
	if (!requirePort(env, info, 0, &port))
		RET_UNDEFINED;
	SteamNetworkingIPAddr address;
	address.Clear();
	address.m_port = port;
	if (info.Length() > 1 && !info[1].IsUndefined()) {
		if (!info[1].IsString() || !address.ParseString(info[1].As<Napi::String>().Utf8Value().c_str())) {
			JS_THROW("host must be an IPv4 or IPv6 address string.");
			RET_UNDEFINED;
		}
		address.m_port = port;
	}
	ISteamNetworkingSockets *value = networkingSockets(env);
	if (value == nullptr)
		RET_UNDEFINED;
	RET_NUM(value->CreateListenSocketIP(address, 0, nullptr));
}

JS_METHOD(connectByIPAddress) {
	NAPI_ENV;
	uint16 port = 0;
	if (!requirePort(env, info, 1, &port))
		RET_UNDEFINED;
	SteamNetworkingIPAddr address;
	if (!requireAddress(env, info, 0, port, &address))
		RET_UNDEFINED;
	ISteamNetworkingSockets *value = networkingSockets(env);
	if (value == nullptr)
		RET_UNDEFINED;
	RET_NUM(value->ConnectByIPAddress(address, 0, nullptr));
}

JS_METHOD(createListenSocketP2P) {
	NAPI_ENV;
	USE_INT32_ARG(0, localVirtualPort, 0);
	ISteamNetworkingSockets *value = networkingSockets(env);
	if (value == nullptr)
		RET_UNDEFINED;
	RET_NUM(value->CreateListenSocketP2P(localVirtualPort, 0, nullptr));
}

JS_METHOD(connectP2P) {
	NAPI_ENV;
	SteamNetworkingIdentity identity;
	if (!requireIdentity(env, info, 0, &identity))
		RET_UNDEFINED;
	USE_INT32_ARG(1, remoteVirtualPort, 0);
	ISteamNetworkingSockets *value = networkingSockets(env);
	if (value == nullptr)
		RET_UNDEFINED;
	RET_NUM(value->ConnectP2P(identity, remoteVirtualPort, 0, nullptr));
}

JS_METHOD(createSocketPair) {
	NAPI_ENV;
	USE_BOOL_ARG(0, useNetworkLoopback, false);
	SteamNetworkingIdentity identity1;
	SteamNetworkingIdentity identity2;
	const SteamNetworkingIdentity *peerIdentity1 = nullptr;
	const SteamNetworkingIdentity *peerIdentity2 = nullptr;
	if (info.Length() > 1 && !IS_ARG_EMPTY(1)) {
		if (!requireIdentity(env, info, 1, &identity1))
			RET_UNDEFINED;
		peerIdentity1 = &identity1;
	}
	if (info.Length() > 2 && !IS_ARG_EMPTY(2)) {
		if (!requireIdentity(env, info, 2, &identity2))
			RET_UNDEFINED;
		peerIdentity2 = &identity2;
	}
	ISteamNetworkingSockets *value = networkingSockets(env);
	if (value == nullptr)
		RET_UNDEFINED;
	HSteamNetConnection connection1 = k_HSteamNetConnection_Invalid;
	HSteamNetConnection connection2 = k_HSteamNetConnection_Invalid;
	if (!value->CreateSocketPair(
	        &connection1, &connection2, useNetworkLoopback, peerIdentity1, peerIdentity2
	    ))
		RET_NULL;
	Napi::Array result = Napi::Array::New(env, 2);
	result.Set(static_cast<uint32>(0), connection1);
	result.Set(static_cast<uint32>(1), connection2);
	RET_VALUE(result);
}

JS_METHOD(configureConnectionLanes) {
	NAPI_ENV;
	REQ_UINT32_ARG(0, connection);
	REQ_INT32_ARG(1, numberOfLanes);
	const int *lanePriorities = nullptr;
	const uint16 *laneWeights = nullptr;
	Napi::Int32Array priorities;
	Napi::Uint16Array weights;
	if (info.Length() > 2 && !IS_ARG_EMPTY(2)) {
		if (!info[2].IsTypedArray() || info[2].As<Napi::TypedArray>().TypedArrayType() != napi_int32_array) {
			JS_THROW("lanePriorities must be an Int32Array, null, or undefined.");
			RET_UNDEFINED;
		}
		priorities = info[2].As<Napi::Int32Array>();
		if (priorities.ElementLength() != static_cast<size_t>(numberOfLanes)) {
			JS_THROW("lanePriorities length must equal numberOfLanes.");
			RET_UNDEFINED;
		}
		lanePriorities = priorities.Data();
	}
	if (info.Length() > 3 && !IS_ARG_EMPTY(3)) {
		if (!info[3].IsTypedArray() || info[3].As<Napi::TypedArray>().TypedArrayType() != napi_uint16_array) {
			JS_THROW("laneWeights must be a Uint16Array, null, or undefined.");
			RET_UNDEFINED;
		}
		weights = info[3].As<Napi::Uint16Array>();
		if (weights.ElementLength() != static_cast<size_t>(numberOfLanes)) {
			JS_THROW("laneWeights length must equal numberOfLanes.");
			RET_UNDEFINED;
		}
		laneWeights = weights.Data();
	}
	ISteamNetworkingSockets *value = networkingSockets(env);
	if (value == nullptr)
		RET_UNDEFINED;
	RET_NUM(value->ConfigureConnectionLanes(connection, numberOfLanes, lanePriorities, laneWeights));
}

JS_METHOD(acceptConnection) {
	NAPI_ENV;
	REQ_UINT32_ARG(0, connection);
	ISteamNetworkingSockets *value = networkingSockets(env);
	if (value == nullptr)
		RET_UNDEFINED;
	RET_NUM(value->AcceptConnection(connection));
}

JS_METHOD(closeConnection) {
	NAPI_ENV;
	REQ_UINT32_ARG(0, connection);
	USE_INT32_ARG(1, reason, 0);
	USE_STR_ARG(2, debug, "");
	USE_BOOL_ARG(3, linger, false);
	ISteamNetworkingSockets *value = networkingSockets(env);
	if (value == nullptr)
		RET_UNDEFINED;
	RET_BOOL(value->CloseConnection(connection, reason, debug.c_str(), linger));
}

JS_METHOD(closeListenSocket) {
	NAPI_ENV;
	REQ_UINT32_ARG(0, listenSocket);
	ISteamNetworkingSockets *value = networkingSockets(env);
	if (value == nullptr)
		RET_UNDEFINED;
	RET_BOOL(value->CloseListenSocket(listenSocket));
}

JS_METHOD(setConnectionUserData) {
	NAPI_ENV;
	REQ_UINT32_ARG(0, connection);
	REQ_INT64_ARG(1, userData);
	ISteamNetworkingSockets *value = networkingSockets(env);
	if (value == nullptr)
		RET_UNDEFINED;
	RET_BOOL(value->SetConnectionUserData(connection, userData));
}

JS_METHOD(getConnectionUserData) {
	NAPI_ENV;
	REQ_UINT32_ARG(0, connection);
	ISteamNetworkingSockets *value = networkingSockets(env);
	if (value == nullptr)
		RET_UNDEFINED;
	RET_NUM(value->GetConnectionUserData(connection));
}

JS_METHOD(sendMessageToConnection) {
	NAPI_ENV;
	REQ_UINT32_ARG(0, connection);
	REQ_BUF_ARG(1, data);
	if (data.Length() > k_cbMaxSteamNetworkingSocketsMessageSizeSend) {
		JS_THROW("data exceeds the maximum GNS message size.");
		RET_UNDEFINED;
	}
	USE_INT32_ARG(2, sendFlags, k_nSteamNetworkingSend_Reliable);
	ISteamNetworkingSockets *value = networkingSockets(env);
	if (value == nullptr)
		RET_UNDEFINED;
	int64 messageNumber = 0;
	Napi::Object result = JS_OBJECT;
	result.Set(
	    "result",
	    static_cast<int32>(value->SendMessageToConnection(
	        connection, data.Data(), static_cast<uint32>(data.Length()), sendFlags, &messageNumber
	    ))
	);
	result.Set("messageNumber", std::to_string(messageNumber));
	RET_VALUE(result);
}

JS_METHOD(sendMessages) {
	NAPI_ENV;
	REQ_ARRAY_ARG(0, source);
	USE_BOOL_ARG(1, deleteFailedMessages, true);
	ISteamNetworkingSockets *value = networkingSockets(env);
	if (value == nullptr)
		RET_UNDEFINED;
	std::vector<message::Value *> wrappers;
	std::vector<SteamNetworkingMessage_t *> messages;
	wrappers.reserve(source.Length());
	messages.reserve(source.Length());
	for (uint32 index = 0; index < source.Length(); ++index) {
		message::Value *wrapper = message::require(env, source.Get(index));
		if (wrapper == nullptr)
			RET_UNDEFINED;
		wrappers.push_back(wrapper);
		messages.push_back(wrapper->native);
	}
	std::vector<int64> results(messages.size());
	value->SendMessages(messages.size(), messages.data(), results.data(), deleteFailedMessages);
	Napi::Array output = Napi::Array::New(env, results.size());
	for (uint32 index = 0; index < results.size(); ++index) {
		if (deleteFailedMessages || results[index] >= 0)
			wrappers[index]->native = nullptr;
		output.Set(index, std::to_string(results[index]));
	}
	RET_VALUE(output);
}

JS_METHOD(receiveMessagesOnConnection) {
	NAPI_ENV;
	REQ_UINT32_ARG(0, connection);
	USE_INT32_ARG(1, maximumMessages, 32);
	if (maximumMessages < 1 || maximumMessages > 256) {
		JS_THROW("maximumMessages must be between 1 and 256.");
		RET_UNDEFINED;
	}
	ISteamNetworkingSockets *value = networkingSockets(env);
	if (value == nullptr)
		RET_UNDEFINED;
	std::vector<SteamNetworkingMessage_t *> messages(maximumMessages);
	int count = value->ReceiveMessagesOnConnection(connection, messages.data(), maximumMessages);
	if (count < 0) {
		JS_THROW("connection is invalid.");
		RET_UNDEFINED;
	}
	RET_VALUE(copyMessages(env, messages.data(), count));
}

JS_METHOD(flushMessagesOnConnection) {
	NAPI_ENV;
	REQ_UINT32_ARG(0, connection);
	ISteamNetworkingSockets *value = networkingSockets(env);
	if (value == nullptr)
		RET_UNDEFINED;
	RET_NUM(value->FlushMessagesOnConnection(connection));
}

JS_METHOD(getListenSocketAddress) {
	NAPI_ENV;
	REQ_UINT32_ARG(0, listenSocket);
	ISteamNetworkingSockets *value = networkingSockets(env);
	if (value == nullptr)
		RET_UNDEFINED;
	SteamNetworkingIPAddr address;
	if (!value->GetListenSocketAddress(listenSocket, &address))
		RET_NULL;
	char text[SteamNetworkingIPAddr::k_cchMaxString] = {};
	address.ToString(text, sizeof(text), true);
	RET_STR(text);
}

JS_METHOD(setConnectionName) {
	NAPI_ENV;
	REQ_UINT32_ARG(0, connection);
	REQ_STR_ARG(1, name);
	ISteamNetworkingSockets *value = networkingSockets(env);
	if (value == nullptr)
		RET_UNDEFINED;
	value->SetConnectionName(connection, name.c_str());
	RET_UNDEFINED;
}

JS_METHOD(getConnectionName) {
	NAPI_ENV;
	REQ_UINT32_ARG(0, connection);
	ISteamNetworkingSockets *value = networkingSockets(env);
	if (value == nullptr)
		RET_UNDEFINED;
	char name[k_cchSteamNetworkingMaxConnectionAppName] = {};
	if (!value->GetConnectionName(connection, name, sizeof(name)))
		RET_NULL;
	RET_STR(name);
}

JS_METHOD(getConnectionInfo) {
	NAPI_ENV;
	REQ_UINT32_ARG(0, connection);
	ISteamNetworkingSockets *value = networkingSockets(env);
	if (value == nullptr)
		RET_UNDEFINED;
	SteamNetConnectionInfo_t connectionInfo = {};
	if (!value->GetConnectionInfo(connection, &connectionInfo))
		RET_NULL;
	char address[SteamNetworkingIPAddr::k_cchMaxString] = {};
	connectionInfo.m_addrRemote.ToString(address, sizeof(address), true);
	Napi::Object result = JS_OBJECT;
	result.Set("connection", connection);
	result.Set("listenSocket", connectionInfo.m_hListenSocket);
	result.Set("state", static_cast<int32>(connectionInfo.m_eState));
	result.Set("endReason", connectionInfo.m_eEndReason);
	result.Set("endDebug", connectionInfo.m_szEndDebug);
	result.Set("description", connectionInfo.m_szConnectionDescription);
	result.Set("flags", connectionInfo.m_nFlags);
	result.Set("remoteAddress", address);
	RET_VALUE(result);
}

JS_METHOD(getConnectionRealTimeStatus) {
	NAPI_ENV;
	REQ_UINT32_ARG(0, connection);
	ISteamNetworkingSockets *value = networkingSockets(env);
	if (value == nullptr)
		RET_UNDEFINED;
	SteamNetConnectionRealTimeStatus_t status = {};
	EResult callResult = value->GetConnectionRealTimeStatus(connection, &status, 0, nullptr);
	Napi::Object result = JS_OBJECT;
	result.Set("result", static_cast<int32>(callResult));
	result.Set("state", static_cast<int32>(status.m_eState));
	result.Set("ping", status.m_nPing);
	result.Set("connectionQualityLocal", status.m_flConnectionQualityLocal);
	result.Set("connectionQualityRemote", status.m_flConnectionQualityRemote);
	result.Set("outPacketsPerSecond", status.m_flOutPacketsPerSec);
	result.Set("outBytesPerSecond", status.m_flOutBytesPerSec);
	result.Set("inPacketsPerSecond", status.m_flInPacketsPerSec);
	result.Set("inBytesPerSecond", status.m_flInBytesPerSec);
	result.Set("sendRateBytesPerSecond", status.m_nSendRateBytesPerSecond);
	result.Set("pendingUnreliable", status.m_cbPendingUnreliable);
	result.Set("pendingReliable", status.m_cbPendingReliable);
	result.Set("sentUnackedReliable", status.m_cbSentUnackedReliable);
	result.Set("queueTime", std::to_string(status.m_usecQueueTime));
	result.Set("maxJitter", status.m_usecMaxJitter);
	RET_VALUE(result);
}

JS_METHOD(getDetailedConnectionStatus) {
	NAPI_ENV;
	REQ_UINT32_ARG(0, connection);
	ISteamNetworkingSockets *value = networkingSockets(env);
	if (value == nullptr)
		RET_UNDEFINED;
	int bufferSize = 1024;
	for (int attempt = 0; attempt < 4; ++attempt) {
		std::vector<char> buffer(bufferSize);
		int result = value->GetDetailedConnectionStatus(connection, buffer.data(), bufferSize);
		if (result < 0)
			RET_NULL;
		if (result == 0)
			RET_STR(buffer.data());
		bufferSize = result;
	}
	JS_THROW("detailed connection status exceeded the maximum diagnostic buffer size.");
	RET_UNDEFINED;
}

JS_METHOD(getIdentity) {
	NAPI_ENV;
	ISteamNetworkingSockets *value = networkingSockets(env);
	if (value == nullptr)
		RET_UNDEFINED;
	SteamNetworkingIdentity identity;
	if (!value->GetIdentity(&identity))
		RET_NULL;
	RET_STR(identityToString(identity));
}

JS_METHOD(initAuthentication) {
	NAPI_ENV;
	ISteamNetworkingSockets *value = networkingSockets(env);
	if (value == nullptr)
		RET_UNDEFINED;
	RET_NUM(value->InitAuthentication());
}

JS_METHOD(getAuthenticationStatus) {
	NAPI_ENV;
	ISteamNetworkingSockets *value = networkingSockets(env);
	if (value == nullptr)
		RET_UNDEFINED;
	SteamNetAuthenticationStatus_t status = {};
	Napi::Object result = JS_OBJECT;
	result.Set("availability", static_cast<int32>(value->GetAuthenticationStatus(&status)));
	result.Set("debug", status.m_debugMsg);
	RET_VALUE(result);
}

JS_METHOD(getCertificateRequest) {
	NAPI_ENV;
	ISteamNetworkingSockets *value = networkingSockets(env);
	if (value == nullptr)
		RET_UNDEFINED;
	SteamNetworkingErrMsg errorMessage = {};
	int size = 0;
	if (!value->GetCertificateRequest(&size, nullptr, errorMessage)) {
		Napi::Object result = JS_OBJECT;
		result.Set("ok", false);
		result.Set("certificateRequest", JS_NULL);
		result.Set("errorMessage", errorMessage);
		RET_VALUE(result);
	}
	std::vector<uint8_t> request(size);
	if (!value->GetCertificateRequest(&size, request.data(), errorMessage)) {
		Napi::Object result = JS_OBJECT;
		result.Set("ok", false);
		result.Set("certificateRequest", JS_NULL);
		result.Set("errorMessage", errorMessage);
		RET_VALUE(result);
	}
	Napi::Object result = JS_OBJECT;
	result.Set("ok", true);
	result.Set("certificateRequest", Napi::Buffer<uint8_t>::Copy(env, request.data(), size));
	result.Set("errorMessage", "");
	RET_VALUE(result);
}

JS_METHOD(setCertificate) {
	NAPI_ENV;
	REQ_BUF_ARG(0, certificate);
	ISteamNetworkingSockets *value = networkingSockets(env);
	if (value == nullptr)
		RET_UNDEFINED;
	SteamNetworkingErrMsg errorMessage = {};
	Napi::Object result = JS_OBJECT;
	result.Set("ok", value->SetCertificate(certificate.Data(), certificate.Length(), errorMessage));
	result.Set("errorMessage", errorMessage);
	RET_VALUE(result);
}

JS_METHOD(resetIdentity) {
	NAPI_ENV;
	SteamNetworkingIdentity identity;
	const SteamNetworkingIdentity *valueIdentity = nullptr;
	if (info.Length() > 0 && !IS_ARG_EMPTY(0)) {
		if (!requireIdentity(env, info, 0, &identity))
			RET_UNDEFINED;
		valueIdentity = &identity;
	}
	ISteamNetworkingSockets *value = networkingSockets(env);
	if (value == nullptr)
		RET_UNDEFINED;
	value->ResetIdentity(valueIdentity);
	RET_UNDEFINED;
}

JS_METHOD(createPollGroup) {
	NAPI_ENV;
	ISteamNetworkingSockets *value = networkingSockets(env);
	if (value == nullptr)
		RET_UNDEFINED;
	RET_NUM(value->CreatePollGroup());
}

JS_METHOD(destroyPollGroup) {
	NAPI_ENV;
	REQ_UINT32_ARG(0, pollGroup);
	ISteamNetworkingSockets *value = networkingSockets(env);
	if (value == nullptr)
		RET_UNDEFINED;
	RET_BOOL(value->DestroyPollGroup(pollGroup));
}

JS_METHOD(setConnectionPollGroup) {
	NAPI_ENV;
	REQ_UINT32_ARG(0, connection);
	REQ_UINT32_ARG(1, pollGroup);
	ISteamNetworkingSockets *value = networkingSockets(env);
	if (value == nullptr)
		RET_UNDEFINED;
	RET_BOOL(value->SetConnectionPollGroup(connection, pollGroup));
}

JS_METHOD(receiveMessagesOnPollGroup) {
	NAPI_ENV;
	REQ_UINT32_ARG(0, pollGroup);
	USE_INT32_ARG(1, maximumMessages, 32);
	if (maximumMessages < 1 || maximumMessages > 256) {
		JS_THROW("maximumMessages must be between 1 and 256.");
		RET_UNDEFINED;
	}
	ISteamNetworkingSockets *value = networkingSockets(env);
	if (value == nullptr)
		RET_UNDEFINED;
	std::vector<SteamNetworkingMessage_t *> messages(maximumMessages);
	int count = value->ReceiveMessagesOnPollGroup(pollGroup, messages.data(), maximumMessages);
	if (count < 0) {
		JS_THROW("pollGroup is invalid.");
		RET_UNDEFINED;
	}
	RET_VALUE(copyMessages(env, messages.data(), count));
}
} // namespace

Napi::Object createNamespace(Napi::Env env) {
	Napi::Object value = JS_OBJECT;
	value.Set("createListenSocketIP", Napi::Function::New(env, createListenSocketIP));
	value.Set("connectByIPAddress", Napi::Function::New(env, connectByIPAddress));
	value.Set("createListenSocketP2P", Napi::Function::New(env, createListenSocketP2P));
	value.Set("connectP2P", Napi::Function::New(env, connectP2P));
	value.Set("createSocketPair", Napi::Function::New(env, createSocketPair));
	value.Set("configureConnectionLanes", Napi::Function::New(env, configureConnectionLanes));
	value.Set("acceptConnection", Napi::Function::New(env, acceptConnection));
	value.Set("closeConnection", Napi::Function::New(env, closeConnection));
	value.Set("closeListenSocket", Napi::Function::New(env, closeListenSocket));
	value.Set("setConnectionUserData", Napi::Function::New(env, setConnectionUserData));
	value.Set("getConnectionUserData", Napi::Function::New(env, getConnectionUserData));
	value.Set("sendMessageToConnection", Napi::Function::New(env, sendMessageToConnection));
	value.Set("sendMessages", Napi::Function::New(env, sendMessages));
	value.Set("receiveMessagesOnConnection", Napi::Function::New(env, receiveMessagesOnConnection));
	value.Set("flushMessagesOnConnection", Napi::Function::New(env, flushMessagesOnConnection));
	value.Set("getListenSocketAddress", Napi::Function::New(env, getListenSocketAddress));
	value.Set("setConnectionName", Napi::Function::New(env, setConnectionName));
	value.Set("getConnectionName", Napi::Function::New(env, getConnectionName));
	value.Set("getConnectionInfo", Napi::Function::New(env, getConnectionInfo));
	value.Set("getConnectionRealTimeStatus", Napi::Function::New(env, getConnectionRealTimeStatus));
	value.Set("getDetailedConnectionStatus", Napi::Function::New(env, getDetailedConnectionStatus));
	value.Set("getIdentity", Napi::Function::New(env, getIdentity));
	value.Set("initAuthentication", Napi::Function::New(env, initAuthentication));
	value.Set("getAuthenticationStatus", Napi::Function::New(env, getAuthenticationStatus));
	value.Set("getCertificateRequest", Napi::Function::New(env, getCertificateRequest));
	value.Set("setCertificate", Napi::Function::New(env, setCertificate));
	value.Set("resetIdentity", Napi::Function::New(env, resetIdentity));
	value.Set("createPollGroup", Napi::Function::New(env, createPollGroup));
	value.Set("destroyPollGroup", Napi::Function::New(env, destroyPollGroup));
	value.Set("setConnectionPollGroup", Napi::Function::New(env, setConnectionPollGroup));
	value.Set("receiveMessagesOnPollGroup", Napi::Function::New(env, receiveMessagesOnPollGroup));
	return value;
}
} // namespace gabenet::sockets
