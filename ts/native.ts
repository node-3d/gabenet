import { createRequire } from 'node:module';
import { getBin } from '@node-3d/addon-tools';
import '@node-3d/deps-gns';
import '@node-3d/segfault';

export type TGabenetInitResult = Readonly<{ ok: boolean; errorMessage: string }>;
export type TConnection = number & { readonly __gabenetConnection: unique symbol };
export type TListenSocket = number & { readonly __gabenetListenSocket: unique symbol };
export type TPollGroup = number & { readonly __gabenetPollGroup: unique symbol };
export type TMessage = object & { readonly __gabenetMessage: unique symbol };
export type TSendResult = Readonly<{ result: number; messageNumber: string }>;
export type TSocketPair = readonly [TConnection, TConnection];
export type TReceivedMessage = Readonly<{
	data: Buffer;
	connection: TConnection;
	identityRemote: string;
	channel: number;
	messageNumber: string;
	receivedAt: string;
}>;
export type TConnectionStatusEvent = Readonly<{
	type: 'connection-status-changed';
	connection: TConnection;
	listenSocket: TListenSocket;
	oldState: number;
	state: number;
	endReason: number;
	endDebug: string;
}>;
export type TMessagesSessionRequestEvent = Readonly<{
	type: 'messages-session-request';
	identityRemote: string;
}>;
export type TMessagesSessionFailedEvent = Readonly<{
	type: 'messages-session-failed';
	identityRemote: string;
	state: number;
	endReason: number;
	endDebug: string;
}>;
export type TAuthenticationStatusEvent = Readonly<{
	type: 'authentication-status-changed';
	availability: number;
	debug: string;
}>;
export type TRelayNetworkStatusEvent = Readonly<{
	type: 'relay-network-status-changed';
	availability: number;
	pingMeasurementInProgress: boolean;
	networkConfigAvailability: number;
	anyRelayAvailability: number;
	debug: string;
}>;
export type TDebugOutputEvent = Readonly<{
	type: 'debug-output';
	level: number;
	message: string;
}>;
export type TGabenetEvent =
	| TConnectionStatusEvent
	| TAuthenticationStatusEvent
	| TRelayNetworkStatusEvent
	| TDebugOutputEvent
	| TMessagesSessionRequestEvent
	| TMessagesSessionFailedEvent;
export type TConnectionInfo = Readonly<{
	connection: TConnection;
	listenSocket: TListenSocket;
	state: number;
	endReason: number;
	endDebug: string;
	description: string;
	flags: number;
	remoteAddress: string;
}>;
export type TConnectionRealTimeStatus = Readonly<{
	result: number;
	state: number;
	ping: number;
	connectionQualityLocal: number;
	connectionQualityRemote: number;
	outPacketsPerSecond: number;
	outBytesPerSecond: number;
	inPacketsPerSecond: number;
	inBytesPerSecond: number;
	sendRateBytesPerSecond: number;
	pendingUnreliable: number;
	pendingReliable: number;
	sentUnackedReliable: number;
	queueTime: string;
	maxJitter: number;
}>;
export type TAuthenticationStatus = Readonly<{ availability: number; debug: string }>;
export type TCertificateRequest = Readonly<{
	ok: boolean;
	certificateRequest: Buffer | null;
	errorMessage: string;
}>;
export type TCertificateResult = Readonly<{ ok: boolean; errorMessage: string }>;
export type TLocalPingLocation = Readonly<{ age: number; location: string }>;
export type TDataCenterPing = Readonly<{ ping: number; viaRelayPop: number }>;
export type TRelayNetworkStatus = Readonly<{
	availability: number;
	pingMeasurementInProgress: boolean;
	networkConfigAvailability: number;
	anyRelayAvailability: number;
	debug: string;
}>;
export type TConfigValue = Readonly<{
	result: number;
	dataType: number;
	value: Buffer | null;
}>;
export type TConfigValueInfo = Readonly<{
	name: string;
	dataType: number;
	scope: number;
}>;
export type TSockets = {
	createListenSocketIP: (port: number, host?: string) => TListenSocket;
	connectByIPAddress: (host: string, port: number) => TConnection;
	createListenSocketP2P: (localVirtualPort?: number) => TListenSocket;
	connectP2P: (identityRemote: string, remoteVirtualPort?: number) => TConnection;
	createSocketPair: (
		useNetworkLoopback?: boolean,
		peerIdentity1?: string | null,
		peerIdentity2?: string | null,
	) => TSocketPair | null;
	configureConnectionLanes: (
		connection: TConnection,
		numberOfLanes: number,
		lanePriorities?: Int32Array | null,
		laneWeights?: Uint16Array | null,
	) => number;
	acceptConnection: (connection: TConnection) => number;
	closeConnection: (
		connection: TConnection,
		reason?: number,
		debug?: string,
		linger?: boolean,
	) => boolean;
	closeListenSocket: (listenSocket: TListenSocket) => boolean;
	setConnectionUserData: (connection: TConnection, userData: number) => boolean;
	getConnectionUserData: (connection: TConnection) => number;
	sendMessageToConnection: (
		connection: TConnection,
		data: Buffer,
		sendFlags?: number,
	) => TSendResult;
	sendMessages: (messages: TMessage[], deleteFailedMessages?: boolean) => string[];
	receiveMessagesOnConnection: (
		connection: TConnection,
		maximumMessages?: number,
	) => TReceivedMessage[];
	flushMessagesOnConnection: (connection: TConnection) => number;
	getListenSocketAddress: (listenSocket: TListenSocket) => string | null;
	setConnectionName: (connection: TConnection, name: string) => void;
	getConnectionName: (connection: TConnection) => string | null;
	getConnectionInfo: (connection: TConnection) => TConnectionInfo | null;
	getConnectionRealTimeStatus: (connection: TConnection) => TConnectionRealTimeStatus;
	getDetailedConnectionStatus: (connection: TConnection) => string | null;
	getIdentity: () => string | null;
	initAuthentication: () => number;
	getAuthenticationStatus: () => TAuthenticationStatus;
	getCertificateRequest: () => TCertificateRequest;
	setCertificate: (certificate: Buffer) => TCertificateResult;
	resetIdentity: (identity?: string | null) => void;
	createPollGroup: () => TPollGroup;
	destroyPollGroup: (pollGroup: TPollGroup) => boolean;
	setConnectionPollGroup: (connection: TConnection, pollGroup: TPollGroup) => boolean;
	receiveMessagesOnPollGroup: (
		pollGroup: TPollGroup,
		maximumMessages?: number,
	) => TReceivedMessage[];
};

export type TSessionConnectionInfo = Readonly<{
	state: number;
	identityRemote: string;
	endReason: number;
	endDebug: string;
	pendingUnreliable: number;
	pendingReliable: number;
	sentUnackedReliable: number;
	queueTime: string;
}>;
export type TMessages = {
	sendMessageToUser: (
		identityRemote: string,
		data: Buffer,
		sendFlags?: number,
		remoteChannel?: number,
	) => number;
	receiveMessagesOnChannel: (
		localChannel: number,
		maximumMessages?: number,
	) => TReceivedMessage[];
	acceptSessionWithUser: (identityRemote: string) => boolean;
	closeSessionWithUser: (identityRemote: string) => boolean;
	closeChannelWithUser: (identityRemote: string, localChannel: number) => boolean;
	getSessionConnectionInfo: (identityRemote: string) => TSessionConnectionInfo;
};
export type TUtils = {
	initRelayNetworkAccess: () => void;
	allocateMessage: (
		data: Buffer,
		connection: TConnection,
		sendFlags?: number,
		lane?: number,
	) => TMessage | null;
	getRelayNetworkStatus: () => TRelayNetworkStatus;
	parseIPAddress: (addressText: string, withPort?: boolean) => string | null;
	parseIdentity: (identityText: string) => string | null;
	getIPv4FakeIPType: (ipv4: number) => number;
	isFakeIPv4: (ipv4: number) => boolean;
	getLocalTimestamp: () => string;
	getLocalPingLocation: () => TLocalPingLocation;
	parsePingLocation: (text: string) => string | null;
	estimatePingTimeBetweenLocations: (location1Text: string, location2Text: string) => number;
	estimatePingTimeFromLocalHost: (locationText: string) => number;
	checkPingDataUpToDate: (maximumAgeSeconds: number) => boolean;
	getPingToDataCenter: (popId: number) => TDataCenterPing;
	getDirectPingToPOP: (popId: number) => number;
	getPOPList: () => number[];
	setGlobalConfigValueInt32: (configValue: number, config: number) => boolean;
	setGlobalConfigValueFloat: (configValue: number, config: number) => boolean;
	setGlobalConfigValueString: (configValue: number, config: string) => boolean;
	setConnectionConfigValueInt32: (
		connection: TConnection,
		configValue: number,
		config: number,
	) => boolean;
	setConnectionConfigValueFloat: (
		connection: TConnection,
		configValue: number,
		config: number,
	) => boolean;
	setConnectionConfigValueString: (
		connection: TConnection,
		configValue: number,
		config: string,
	) => boolean;
	getConfigValue: (configValue: number, scope: number, scopeObject: number) => TConfigValue;
	getConfigValueInfo: (configValue: number) => TConfigValueInfo | null;
	iterateGenericEditableConfigValues: (
		currentConfigValue?: number,
		enumerateDevVars?: boolean,
	) => number;
	setDebugOutputLevel: (level: number) => void;
};

type TNative = {
	init: () => TGabenetInitResult;
	shutdown: () => void;
	isInitialized: () => boolean;
	runCallbacks: () => void;
	pollEvents: () => TGabenetEvent[];
	sockets: TSockets;
	messages: TMessages;
	utils: TUtils;
};

const loadAddon = createRequire(import.meta.url);
export const native = loadAddon(`../${getBin()}/gabenet.node`) as TNative;
