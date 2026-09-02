#include "events.hpp"

#include <mutex>
#include <string>
#include <utility>
#include <vector>

namespace gabenet::events {
namespace {
struct ConnectionEvent {
	HSteamNetConnection connection;
	HSteamListenSocket listenSocket;
	int oldState;
	int state;
	int endReason;
	std::string endDebug;
};

enum class EventType {
	ConnectionStatusChanged,
	AuthenticationStatusChanged,
	RelayNetworkStatusChanged,
	DebugOutput,
	MessagesSessionRequest,
	MessagesSessionFailed,
};

struct Event {
	EventType type;
	ConnectionEvent connection = {};
	std::string identityRemote;
	int availability = 0;
	bool pingMeasurementInProgress = false;
	int networkConfigAvailability = 0;
	int anyRelayAvailability = 0;
	std::string debug;
};

std::vector<Event> queuedEvents;
std::mutex queuedEventsMutex;

void onConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t *callback) {
	std::lock_guard<std::mutex> lock(queuedEventsMutex);
	queuedEvents.push_back(
	    {
	        EventType::ConnectionStatusChanged,
	        {
	            callback->m_hConn,
	            callback->m_info.m_hListenSocket,
	            callback->m_eOldState,
	            callback->m_info.m_eState,
	            callback->m_info.m_eEndReason,
	            callback->m_info.m_szEndDebug,
	        },
	    }
	);
}

void onMessagesSessionRequest(SteamNetworkingMessagesSessionRequest_t *callback) {
	std::lock_guard<std::mutex> lock(queuedEventsMutex);
	queuedEvents.push_back(
	    { EventType::MessagesSessionRequest, {}, identityToString(callback->m_identityRemote) }
	);
}

void onAuthenticationStatusChanged(SteamNetAuthenticationStatus_t *callback) {
	std::lock_guard<std::mutex> lock(queuedEventsMutex);
	Event event = { EventType::AuthenticationStatusChanged };
	event.availability = callback->m_eAvail;
	event.debug = callback->m_debugMsg;
	queuedEvents.push_back(std::move(event));
}

void onRelayNetworkStatusChanged(SteamRelayNetworkStatus_t *callback) {
	std::lock_guard<std::mutex> lock(queuedEventsMutex);
	Event event = { EventType::RelayNetworkStatusChanged };
	event.availability = callback->m_eAvail;
	event.pingMeasurementInProgress = callback->m_bPingMeasurementInProgress != 0;
	event.networkConfigAvailability = callback->m_eAvailNetworkConfig;
	event.anyRelayAvailability = callback->m_eAvailAnyRelay;
	event.debug = callback->m_debugMsg;
	queuedEvents.push_back(std::move(event));
}

void onMessagesSessionFailed(SteamNetworkingMessagesSessionFailed_t *callback) {
	std::lock_guard<std::mutex> lock(queuedEventsMutex);
	queuedEvents.push_back(
	    {
	        EventType::MessagesSessionFailed,
	        {
	            0,
	            callback->m_info.m_hListenSocket,
	            0,
	            callback->m_info.m_eState,
	            callback->m_info.m_eEndReason,
	            callback->m_info.m_szEndDebug,
	        },
	        identityToString(callback->m_info.m_identityRemote),
	    }
	);
}

void onDebugOutput(ESteamNetworkingSocketsDebugOutputType level, const char *message) {
	std::lock_guard<std::mutex> lock(queuedEventsMutex);
	Event event = { EventType::DebugOutput };
	event.availability = level;
	event.debug = message;
	queuedEvents.push_back(std::move(event));
}
} // namespace

void registerCallbacks() {
	ISteamNetworkingUtils *utils = SteamNetworkingUtils_Lib();
	utils->SetGlobalCallback_SteamNetConnectionStatusChanged(onConnectionStatusChanged);
	utils->SetGlobalCallback_SteamNetAuthenticationStatusChanged(onAuthenticationStatusChanged);
	utils->SetGlobalCallback_SteamRelayNetworkStatusChanged(onRelayNetworkStatusChanged);
	utils->SetGlobalCallback_MessagesSessionRequest(onMessagesSessionRequest);
	utils->SetGlobalCallback_MessagesSessionFailed(onMessagesSessionFailed);
}

void clear() {
	std::lock_guard<std::mutex> lock(queuedEventsMutex);
	queuedEvents.clear();
}

void setDebugOutputLevel(ESteamNetworkingSocketsDebugOutputType level) {
	SteamNetworkingUtils_Lib()->SetDebugOutputFunction(level, onDebugOutput);
}

JS_METHOD(pollEvents) {
	NAPI_ENV;
	std::lock_guard<std::mutex> lock(queuedEventsMutex);
	Napi::Array result = Napi::Array::New(env, queuedEvents.size());
	for (size_t index = 0; index < queuedEvents.size(); ++index) {
		const Event &event = queuedEvents[index];
		Napi::Object value = JS_OBJECT;
		if (event.type == EventType::ConnectionStatusChanged) {
			value.Set("type", "connection-status-changed");
			value.Set("connection", event.connection.connection);
			value.Set("listenSocket", event.connection.listenSocket);
			value.Set("oldState", event.connection.oldState);
			value.Set("state", event.connection.state);
			value.Set("endReason", event.connection.endReason);
			value.Set("endDebug", event.connection.endDebug);
		} else if (event.type == EventType::AuthenticationStatusChanged) {
			value.Set("type", "authentication-status-changed");
			value.Set("availability", event.availability);
			value.Set("debug", event.debug);
		} else if (event.type == EventType::RelayNetworkStatusChanged) {
			value.Set("type", "relay-network-status-changed");
			value.Set("availability", event.availability);
			value.Set("pingMeasurementInProgress", event.pingMeasurementInProgress);
			value.Set("networkConfigAvailability", event.networkConfigAvailability);
			value.Set("anyRelayAvailability", event.anyRelayAvailability);
			value.Set("debug", event.debug);
		} else if (event.type == EventType::DebugOutput) {
			value.Set("type", "debug-output");
			value.Set("level", event.availability);
			value.Set("message", event.debug);
		} else if (event.type == EventType::MessagesSessionRequest) {
			value.Set("type", "messages-session-request");
			value.Set("identityRemote", event.identityRemote);
		} else {
			value.Set("type", "messages-session-failed");
			value.Set("identityRemote", event.identityRemote);
			value.Set("state", event.connection.state);
			value.Set("endReason", event.connection.endReason);
			value.Set("endDebug", event.connection.endDebug);
		}
		result.Set(index, value);
	}
	queuedEvents.clear();
	RET_VALUE(result);
}
} // namespace gabenet::events
