# GameNetworkingSockets API checklist

Source: GameNetworkingSockets 1.6 headers in `@node-3d/deps-gns`.

## Library lifecycle

- [x] `GameNetworkingSockets_Init`
- [x] `GameNetworkingSockets_Kill`
- [x] `ISteamNetworkingSockets::RunCallbacks`
- [ ] `SteamNetworkingSockets_SetCustomMemoryAllocator` — excluded process-global hook
- [ ] `SteamNetworkingSockets_SetLockWaitWarningThreshold` — excluded process-global hook
- [ ] `SteamNetworkingSockets_SetLockAcquiredCallback` — excluded process-global hook
- [ ] `SteamNetworkingSockets_SetLockHeldCallback` — excluded process-global hook
- [ ] `SteamNetworkingSockets_SetServiceThreadInitCallback` — excluded process-global hook

## Socket creation and connection management

- [x] `CreateListenSocketIP`
- [x] `ConnectByIPAddress`
- [x] `CreateListenSocketP2P`
- [x] `ConnectP2P`
- [x] `AcceptConnection`
- [x] `CloseConnection`
- [x] `CloseListenSocket`
- [x] `SetConnectionUserData` / `GetConnectionUserData`
- [x] `SetConnectionName` / `GetConnectionName`
- [x] `GetConnectionInfo`
- [x] `GetConnectionRealTimeStatus`
- [x] `GetDetailedConnectionStatus`
- [x] `GetListenSocketAddress`
- [x] `CreateSocketPair`
- [x] `ConfigureConnectionLanes`
- [x] `GetIdentity`

## Connection messages and polling

- [x] `SendMessageToConnection`
- [x] `SendMessages`
- [x] `FlushMessagesOnConnection`
- [x] `ReceiveMessagesOnConnection`
- [x] `CreatePollGroup` / `DestroyPollGroup`
- [x] `SetConnectionPollGroup`
- [x] `ReceiveMessagesOnPollGroup`

## Connectionless messages

- [x] `ISteamNetworkingMessages::SendMessageToUser`
- [x] `ReceiveMessagesOnChannel`
- [x] `AcceptSessionWithUser`
- [x] `CloseSessionWithUser`
- [x] `CloseChannelWithUser`
- [x] `GetSessionConnectionInfo`

## Events

- [x] `SteamNetConnectionStatusChangedCallback_t`
- [x] `SteamNetAuthenticationStatus_t`
- [x] `SteamRelayNetworkStatus_t`
- [x] `SteamNetworkingMessagesSessionRequest_t`
- [x] `SteamNetworkingMessagesSessionFailed_t`
- [ ] `SteamNetworkingFakeIPResult_t` — unavailable in standalone public headers
- [x] debug-output callback

## Identity, address, and configuration utilities

- [x] IP address parse/stringify/FakeIP type
- [x] identity parse/stringify
- [ ] `SetConfigValue` — pointer-capable generic setter excluded; typed safe setters are available
- [x] `GetConfigValue`
- [x] `GetConfigValueInfo`
- [x] `IterateGenericEditableConfigValues`
- [x] typed global and connection config setters
- [x] `GetLocalTimestamp`
- [x] `AllocateMessage`

## Authentication, relay, and ping services

- [x] `InitAuthentication` / `GetAuthenticationStatus`
- [x] `GetCertificateRequest` / `SetCertificate` / `ResetIdentity`
- [x] `InitRelayNetworkAccess` / `GetRelayNetworkStatus`
- [x] ping location get/parse/stringify/estimate/check freshness
- [x] POP count/list/direct ping/relay ping
- [ ] `ReceivedRelayAuthTicket` / `FindRelayAuthTicketForServer` — unavailable in standalone SDR build

## Hosted dedicated servers

- [ ] hosted dedicated-server API — unavailable in standalone SDR build

## P2P custom signalling

- [ ] `ConnectP2PCustomSignaling`
- [ ] `ReceivedP2PCustomSignal`
- [ ] `ISteamNetworkingConnectionSignaling::SendSignal`
- [ ] `ISteamNetworkingConnectionSignaling::Release`
- [ ] `ISteamNetworkingSignalingRecvContext::OnConnectRequest`
- [ ] `ISteamNetworkingSignalingRecvContext::SendRejectionSignal`

## FakeIP and FakeUDP

- [ ] FakeIP and FakeUDP API — unavailable in standalone public headers/build

## Not in the standalone public contract

- [ ] Steamworks `SteamNetworkingSockets_SteamAPI` and `SteamNetworkingMessages_SteamAPI` accessors
- [ ] Steamworks game-server accessors
