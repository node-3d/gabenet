import { native } from './native.ts';

export type {
	TAuthenticationStatus,
	TAuthenticationStatusEvent,
	TConnection,
	TCertificateRequest,
	TCertificateResult,
	TConfigValue,
	TConfigValueInfo,
	TDebugOutputEvent,
	TDataCenterPing,
	TConnectionInfo,
	TConnectionRealTimeStatus,
	TConnectionStatusEvent,
	TGabenetInitResult,
	TGabenetEvent,
	TListenSocket,
	TMessage,
	TLocalPingLocation,
	TMessages,
	TMessagesSessionFailedEvent,
	TMessagesSessionRequestEvent,
	TPollGroup,
	TReceivedMessage,
	TRelayNetworkStatus,
	TRelayNetworkStatusEvent,
	TSessionConnectionInfo,
	TSendResult,
	TSocketPair,
	TSockets,
	TUtils,
} from './native.ts';

export const { init, shutdown, isInitialized, runCallbacks, pollEvents, sockets, messages, utils } =
	native;
