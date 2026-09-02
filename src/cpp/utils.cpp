#include "utils.hpp"

#include "events.hpp"
#include "message.hpp"

#include <vector>

namespace gabenet::utils {
namespace {
JS_METHOD(initRelayNetworkAccess) {
	NAPI_ENV;
	ISteamNetworkingUtils *value = networkingUtils(env);
	if (value == nullptr)
		RET_UNDEFINED;
	value->InitRelayNetworkAccess();
	RET_UNDEFINED;
}

JS_METHOD(allocateMessage) {
	NAPI_ENV;
	REQ_BUF_ARG(0, data);
	REQ_UINT32_ARG(1, connection);
	USE_INT32_ARG(2, sendFlags, k_nSteamNetworkingSend_Reliable);
	USE_INT32_ARG(3, lane, 0);
	ISteamNetworkingUtils *value = networkingUtils(env);
	if (value == nullptr)
		RET_UNDEFINED;
	SteamNetworkingMessage_t *message = value->AllocateMessage(data.Length());
	if (message == nullptr)
		RET_NULL;
	memcpy(message->m_pData, data.Data(), data.Length());
	message->m_cbSize = static_cast<int>(data.Length());
	message->m_conn = connection;
	message->m_nFlags = sendFlags;
	message->m_idxLane = static_cast<int16>(lane);
	RET_VALUE(message::create(env, message));
}

JS_METHOD(getRelayNetworkStatus) {
	NAPI_ENV;
	ISteamNetworkingUtils *value = networkingUtils(env);
	if (value == nullptr)
		RET_UNDEFINED;
	SteamRelayNetworkStatus_t status = {};
	Napi::Object result = JS_OBJECT;
	result.Set("availability", static_cast<int32>(value->GetRelayNetworkStatus(&status)));
	result.Set("pingMeasurementInProgress", status.m_bPingMeasurementInProgress != 0);
	result.Set("networkConfigAvailability", static_cast<int32>(status.m_eAvailNetworkConfig));
	result.Set("anyRelayAvailability", static_cast<int32>(status.m_eAvailAnyRelay));
	result.Set("debug", status.m_debugMsg);
	RET_VALUE(result);
}

JS_METHOD(parseIPAddress) {
	NAPI_ENV;
	REQ_STR_ARG(0, addressText);
	USE_BOOL_ARG(1, withPort, true);
	SteamNetworkingIPAddr address;
	address.Clear();
	if (!address.ParseString(addressText.c_str()))
		RET_NULL;
	char result[SteamNetworkingIPAddr::k_cchMaxString] = {};
	address.ToString(result, sizeof(result), withPort);
	RET_STR(result);
}

JS_METHOD(parseIdentity) {
	NAPI_ENV;
	REQ_STR_ARG(0, identityText);
	SteamNetworkingIdentity identity;
	identity.Clear();
	if (!identity.ParseString(identityText.c_str()) || identity.IsInvalid())
		RET_NULL;
	RET_STR(identityToString(identity));
}

JS_METHOD(getIPv4FakeIPType) {
	NAPI_ENV;
	REQ_UINT32_ARG(0, ipv4);
	ISteamNetworkingUtils *value = networkingUtils(env);
	if (value == nullptr)
		RET_UNDEFINED;
	RET_NUM(value->GetIPv4FakeIPType(ipv4));
}

JS_METHOD(isFakeIPv4) {
	NAPI_ENV;
	REQ_UINT32_ARG(0, ipv4);
	ISteamNetworkingUtils *value = networkingUtils(env);
	if (value == nullptr)
		RET_UNDEFINED;
	RET_BOOL(value->IsFakeIPv4(ipv4));
}

JS_METHOD(getLocalTimestamp) {
	NAPI_ENV;
	ISteamNetworkingUtils *value = networkingUtils(env);
	if (value == nullptr)
		RET_UNDEFINED;
	RET_STR(std::to_string(value->GetLocalTimestamp()));
}

JS_METHOD(getLocalPingLocation) {
	NAPI_ENV;
	ISteamNetworkingUtils *value = networkingUtils(env);
	if (value == nullptr)
		RET_UNDEFINED;
	SteamNetworkPingLocation_t location = {};
	char text[k_cchMaxSteamNetworkingPingLocationString] = {};
	Napi::Object result = JS_OBJECT;
	result.Set("age", value->GetLocalPingLocation(location));
	value->ConvertPingLocationToString(location, text, sizeof(text));
	result.Set("location", text);
	RET_VALUE(result);
}

JS_METHOD(parsePingLocation) {
	NAPI_ENV;
	REQ_STR_ARG(0, text);
	ISteamNetworkingUtils *value = networkingUtils(env);
	if (value == nullptr)
		RET_UNDEFINED;
	SteamNetworkPingLocation_t location = {};
	if (!value->ParsePingLocationString(text.c_str(), location))
		RET_NULL;
	char canonical[k_cchMaxSteamNetworkingPingLocationString] = {};
	value->ConvertPingLocationToString(location, canonical, sizeof(canonical));
	RET_STR(canonical);
}

JS_METHOD(estimatePingTimeBetweenLocations) {
	NAPI_ENV;
	REQ_STR_ARG(0, location1Text);
	REQ_STR_ARG(1, location2Text);
	ISteamNetworkingUtils *value = networkingUtils(env);
	if (value == nullptr)
		RET_UNDEFINED;
	SteamNetworkPingLocation_t location1 = {};
	SteamNetworkPingLocation_t location2 = {};
	if (!value->ParsePingLocationString(location1Text.c_str(), location1) ||
	    !value->ParsePingLocationString(location2Text.c_str(), location2)) {
		JS_THROW("locations must be valid GNS ping-location strings.");
		RET_UNDEFINED;
	}
	RET_NUM(value->EstimatePingTimeBetweenTwoLocations(location1, location2));
}

JS_METHOD(estimatePingTimeFromLocalHost) {
	NAPI_ENV;
	REQ_STR_ARG(0, locationText);
	ISteamNetworkingUtils *value = networkingUtils(env);
	if (value == nullptr)
		RET_UNDEFINED;
	SteamNetworkPingLocation_t location = {};
	if (!value->ParsePingLocationString(locationText.c_str(), location)) {
		JS_THROW("location must be a valid GNS ping-location string.");
		RET_UNDEFINED;
	}
	RET_NUM(value->EstimatePingTimeFromLocalHost(location));
}

JS_METHOD(checkPingDataUpToDate) {
	NAPI_ENV;
	REQ_FLOAT_ARG(0, maximumAgeSeconds);
	ISteamNetworkingUtils *value = networkingUtils(env);
	if (value == nullptr)
		RET_UNDEFINED;
	RET_BOOL(value->CheckPingDataUpToDate(maximumAgeSeconds));
}

JS_METHOD(getPingToDataCenter) {
	NAPI_ENV;
	REQ_UINT32_ARG(0, popId);
	ISteamNetworkingUtils *value = networkingUtils(env);
	if (value == nullptr)
		RET_UNDEFINED;
	SteamNetworkingPOPID viaRelayPop = 0;
	Napi::Object result = JS_OBJECT;
	result.Set("ping", value->GetPingToDataCenter(popId, &viaRelayPop));
	result.Set("viaRelayPop", viaRelayPop);
	RET_VALUE(result);
}

JS_METHOD(getDirectPingToPOP) {
	NAPI_ENV;
	REQ_UINT32_ARG(0, popId);
	ISteamNetworkingUtils *value = networkingUtils(env);
	if (value == nullptr)
		RET_UNDEFINED;
	RET_NUM(value->GetDirectPingToPOP(popId));
}

JS_METHOD(getPOPList) {
	NAPI_ENV;
	ISteamNetworkingUtils *value = networkingUtils(env);
	if (value == nullptr)
		RET_UNDEFINED;
	int count = value->GetPOPCount();
	std::vector<SteamNetworkingPOPID> popIds(count);
	count = value->GetPOPList(popIds.data(), count);
	Napi::Array result = Napi::Array::New(env, count);
	for (int index = 0; index < count; ++index)
		result.Set(index, popIds[index]);
	RET_VALUE(result);
}

JS_METHOD(setGlobalConfigValueInt32) {
	NAPI_ENV;
	REQ_INT32_ARG(0, configValue);
	REQ_INT32_ARG(1, config);
	ISteamNetworkingUtils *value = networkingUtils(env);
	if (value == nullptr)
		RET_UNDEFINED;
	RET_BOOL(value->SetGlobalConfigValueInt32(static_cast<ESteamNetworkingConfigValue>(configValue), config));
}

JS_METHOD(setGlobalConfigValueFloat) {
	NAPI_ENV;
	REQ_INT32_ARG(0, configValue);
	REQ_FLOAT_ARG(1, config);
	ISteamNetworkingUtils *value = networkingUtils(env);
	if (value == nullptr)
		RET_UNDEFINED;
	RET_BOOL(value->SetGlobalConfigValueFloat(static_cast<ESteamNetworkingConfigValue>(configValue), config));
}

JS_METHOD(setGlobalConfigValueString) {
	NAPI_ENV;
	REQ_INT32_ARG(0, configValue);
	REQ_STR_ARG(1, config);
	ISteamNetworkingUtils *value = networkingUtils(env);
	if (value == nullptr)
		RET_UNDEFINED;
	RET_BOOL(value->SetGlobalConfigValueString(
	    static_cast<ESteamNetworkingConfigValue>(configValue), config.c_str()
	));
}

JS_METHOD(setConnectionConfigValueInt32) {
	NAPI_ENV;
	REQ_UINT32_ARG(0, connection);
	REQ_INT32_ARG(1, configValue);
	REQ_INT32_ARG(2, config);
	ISteamNetworkingUtils *value = networkingUtils(env);
	if (value == nullptr)
		RET_UNDEFINED;
	RET_BOOL(value->SetConnectionConfigValueInt32(
	    connection, static_cast<ESteamNetworkingConfigValue>(configValue), config
	));
}

JS_METHOD(setConnectionConfigValueFloat) {
	NAPI_ENV;
	REQ_UINT32_ARG(0, connection);
	REQ_INT32_ARG(1, configValue);
	REQ_FLOAT_ARG(2, config);
	ISteamNetworkingUtils *value = networkingUtils(env);
	if (value == nullptr)
		RET_UNDEFINED;
	RET_BOOL(value->SetConnectionConfigValueFloat(
	    connection, static_cast<ESteamNetworkingConfigValue>(configValue), config
	));
}

JS_METHOD(setConnectionConfigValueString) {
	NAPI_ENV;
	REQ_UINT32_ARG(0, connection);
	REQ_INT32_ARG(1, configValue);
	REQ_STR_ARG(2, config);
	ISteamNetworkingUtils *value = networkingUtils(env);
	if (value == nullptr)
		RET_UNDEFINED;
	RET_BOOL(value->SetConnectionConfigValueString(
	    connection, static_cast<ESteamNetworkingConfigValue>(configValue), config.c_str()
	));
}

JS_METHOD(getConfigValue) {
	NAPI_ENV;
	REQ_INT32_ARG(0, configValue);
	REQ_INT32_ARG(1, scope);
	REQ_INT64_ARG(2, scopeObject);
	ISteamNetworkingUtils *value = networkingUtils(env);
	if (value == nullptr)
		RET_UNDEFINED;

	ESteamNetworkingConfigDataType dataType = k_ESteamNetworkingConfig_Int32;
	size_t size = 0;
	ESteamNetworkingGetConfigValueResult result = value->GetConfigValue(
	    static_cast<ESteamNetworkingConfigValue>(configValue),
	    static_cast<ESteamNetworkingConfigScope>(scope),
	    static_cast<intptr_t>(scopeObject),
	    &dataType,
	    nullptr,
	    &size
	);

	Napi::Object output = JS_OBJECT;
	output.Set("result", static_cast<int32>(result));
	output.Set("dataType", static_cast<int32>(dataType));
	if (result != k_ESteamNetworkingGetConfigValue_BufferTooSmall) {
		output.Set("value", JS_NULL);
		RET_VALUE(output);
	}

	std::vector<uint8_t> buffer(size);
	result = value->GetConfigValue(
	    static_cast<ESteamNetworkingConfigValue>(configValue),
	    static_cast<ESteamNetworkingConfigScope>(scope),
	    static_cast<intptr_t>(scopeObject),
	    &dataType,
	    buffer.data(),
	    &size
	);
	output.Set("result", static_cast<int32>(result));
	output.Set("dataType", static_cast<int32>(dataType));
	if (result == k_ESteamNetworkingGetConfigValue_OK ||
	    result == k_ESteamNetworkingGetConfigValue_OKInherited) {
		output.Set("value", Napi::Buffer<uint8_t>::Copy(env, buffer.data(), size));
	} else {
		output.Set("value", JS_NULL);
	}
	RET_VALUE(output);
}

JS_METHOD(getConfigValueInfo) {
	NAPI_ENV;
	REQ_INT32_ARG(0, configValue);
	ISteamNetworkingUtils *value = networkingUtils(env);
	if (value == nullptr)
		RET_UNDEFINED;
	ESteamNetworkingConfigDataType dataType = k_ESteamNetworkingConfig_Int32;
	ESteamNetworkingConfigScope scope = k_ESteamNetworkingConfig_Global;
	const char *name =
	    value->GetConfigValueInfo(static_cast<ESteamNetworkingConfigValue>(configValue), &dataType, &scope);
	if (name == nullptr)
		RET_NULL;
	Napi::Object output = JS_OBJECT;
	output.Set("name", name);
	output.Set("dataType", static_cast<int32>(dataType));
	output.Set("scope", static_cast<int32>(scope));
	RET_VALUE(output);
}

JS_METHOD(iterateGenericEditableConfigValues) {
	NAPI_ENV;
	USE_INT32_ARG(0, currentConfigValue, k_ESteamNetworkingConfig_Invalid);
	USE_BOOL_ARG(1, enumerateDevVars, false);
	ISteamNetworkingUtils *value = networkingUtils(env);
	if (value == nullptr)
		RET_UNDEFINED;
	RET_NUM(value->IterateGenericEditableConfigValues(
	    static_cast<ESteamNetworkingConfigValue>(currentConfigValue), enumerateDevVars
	));
}

JS_METHOD(setDebugOutputLevel) {
	NAPI_ENV;
	REQ_INT32_ARG(0, level);
	if (networkingUtils(env) == nullptr)
		RET_UNDEFINED;
	events::setDebugOutputLevel(static_cast<ESteamNetworkingSocketsDebugOutputType>(level));
	RET_UNDEFINED;
}
} // namespace

Napi::Object createNamespace(Napi::Env env) {
	Napi::Object value = JS_OBJECT;
	value.Set("initRelayNetworkAccess", Napi::Function::New(env, initRelayNetworkAccess));
	value.Set("allocateMessage", Napi::Function::New(env, allocateMessage));
	value.Set("getRelayNetworkStatus", Napi::Function::New(env, getRelayNetworkStatus));
	value.Set("parseIPAddress", Napi::Function::New(env, parseIPAddress));
	value.Set("parseIdentity", Napi::Function::New(env, parseIdentity));
	value.Set("getIPv4FakeIPType", Napi::Function::New(env, getIPv4FakeIPType));
	value.Set("isFakeIPv4", Napi::Function::New(env, isFakeIPv4));
	value.Set("getLocalTimestamp", Napi::Function::New(env, getLocalTimestamp));
	value.Set("getLocalPingLocation", Napi::Function::New(env, getLocalPingLocation));
	value.Set("parsePingLocation", Napi::Function::New(env, parsePingLocation));
	value.Set("estimatePingTimeBetweenLocations", Napi::Function::New(env, estimatePingTimeBetweenLocations));
	value.Set("estimatePingTimeFromLocalHost", Napi::Function::New(env, estimatePingTimeFromLocalHost));
	value.Set("checkPingDataUpToDate", Napi::Function::New(env, checkPingDataUpToDate));
	value.Set("getPingToDataCenter", Napi::Function::New(env, getPingToDataCenter));
	value.Set("getDirectPingToPOP", Napi::Function::New(env, getDirectPingToPOP));
	value.Set("getPOPList", Napi::Function::New(env, getPOPList));
	value.Set("setGlobalConfigValueInt32", Napi::Function::New(env, setGlobalConfigValueInt32));
	value.Set("setGlobalConfigValueFloat", Napi::Function::New(env, setGlobalConfigValueFloat));
	value.Set("setGlobalConfigValueString", Napi::Function::New(env, setGlobalConfigValueString));
	value.Set("setConnectionConfigValueInt32", Napi::Function::New(env, setConnectionConfigValueInt32));
	value.Set("setConnectionConfigValueFloat", Napi::Function::New(env, setConnectionConfigValueFloat));
	value.Set("setConnectionConfigValueString", Napi::Function::New(env, setConnectionConfigValueString));
	value.Set("getConfigValue", Napi::Function::New(env, getConfigValue));
	value.Set("getConfigValueInfo", Napi::Function::New(env, getConfigValueInfo));
	value.Set(
	    "iterateGenericEditableConfigValues", Napi::Function::New(env, iterateGenericEditableConfigValues)
	);
	value.Set("setDebugOutputLevel", Napi::Function::New(env, setDebugOutputLevel));
	return value;
}
} // namespace gabenet::utils
