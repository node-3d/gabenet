# GameNetworkingSockets for Node.js

This is a part of [Node3D](https://github.com/node-3d) project.

[![NPM](https://badge.fury.io/js/@node-3d%2Fgabenet.svg)](https://badge.fury.io/js/@node-3d/gabenet)
[![Lint](https://github.com/node-3d/gabenet/actions/workflows/lint.yml/badge.svg)](https://github.com/node-3d/gabenet/actions/workflows/lint.yml)
[![Test](https://github.com/node-3d/gabenet/actions/workflows/test.yml/badge.svg)](https://github.com/node-3d/gabenet/actions/workflows/test.yml)

```bash
npm install @node-3d/gabenet
```

`@node-3d/gabenet` is the Node.js native addon for the standalone
[GameNetworkingSockets](https://github.com/ValveSoftware/GameNetworkingSockets)
transport. It does not use Steamworks and does not require a running Steam client.

The initial API exposes the standalone library lifecycle. It is the common
foundation for ordinary client/server UDP sockets and future P2P or custom
signalling interfaces.

```ts
import { init, isInitialized, runCallbacks, shutdown } from '@node-3d/gabenet';

const result = init();
if (!result.ok) throw new Error(result.errorMessage);
runCallbacks();
console.log(isInitialized());
shutdown();
```

`init()` returns `{ ok, errorMessage }`; it does not throw for a library initialization failure.
Calling `init()` again while initialized succeeds without reinitializing GNS. `shutdown()` is safe
to call when it is not initialized.

## UDP sockets

`sockets` is GNS's connection-oriented, message-based transport over ordinary IPv4 or IPv6 UDP.
Call `runCallbacks()` regularly, then read `pollEvents()` and promptly accept incoming connections.

```ts
import { pollEvents, runCallbacks, sockets } from '@node-3d/gabenet';

const listenSocket = sockets.createListenSocketIP(30000, '0.0.0.0');

runCallbacks();
for (const event of pollEvents()) {
	if (event.type === 'connection-status-changed' && event.listenSocket === listenSocket) {
		sockets.acceptConnection(event.connection);
	}
}
```

Use `connectByIPAddress(host, port)` on clients. `sendMessageToConnection()` accepts a `Buffer` and
defaults to reliable delivery; `receiveMessagesOnConnection()` returns copied `Buffer` payloads.
Use `createPollGroup()` and `receiveMessagesOnPollGroup()` to collect messages from multiple
connections. Connection names and basic status are available through `setConnectionName()`,
`getConnectionName()`, and `getConnectionInfo()`. Attach an application safe-integer ID with
`setConnectionUserData()` / `getConnectionUserData()`. `getConnectionRealTimeStatus()` returns the
current no-lane throughput and queue snapshot, while `getDetailedConnectionStatus()` provides GNS's
diagnostic text. `getIdentity()` returns the local GNS identity when one is available. Close connections,
poll groups, and listen sockets explicitly before `shutdown()`.

`createSocketPair()` creates two already-connected local endpoints for local IPC or testing. Configure
independent outbound message lanes with `configureConnectionLanes(connection, numberOfLanes,
lanePriorities, laneWeights)`, using `Int32Array` priorities and `Uint16Array` weights; pass `null` for
either array to use GNS's default for that property.

For GNS's no-copy batch path, `utils.allocateMessage(data, connection, sendFlags?, lane?)` creates an
opaque native message handle. Pass one or more handles to `sockets.sendMessages()`; after GNS accepts
a message, the handle must not be reused. Unsent handles remain owned by JavaScript only when
`deleteFailedMessages` is `false`.

## Authentication and relay status

Use `sockets.initAuthentication()` and `sockets.getAuthenticationStatus()` for authentication readiness.
Use `utils.initRelayNetworkAccess()` before anticipated relay or P2P use, then query
`utils.getRelayNetworkStatus()`. Changes are delivered through `pollEvents()` as
`authentication-status-changed` and `relay-network-status-changed` events.

For standalone identity provisioning, `sockets.getCertificateRequest()` returns the opaque request to
send to a certificate issuer, `sockets.setCertificate()` accepts that issuer's certificate `Buffer`, and
`sockets.resetIdentity(identity?)` discards the active identity and closes open connections.

## P2P sockets

`sockets.createListenSocketP2P(localVirtualPort?)` and
`sockets.connectP2P(identityRemote, remoteVirtualPort?)` provide GNS's default-signalling P2P flow.
Initialize relay access before using them. A successful P2P connection also depends on compatible peer
identities and the rendezvous/signalling service configured for the standalone GNS deployment.

## Utilities

`utils.parseIPAddress()` and `utils.parseIdentity()` validate and return canonical GNS strings, or
`null` for invalid input. `utils.getIPv4FakeIPType()` / `utils.isFakeIPv4()` inspect a host-order IPv4
`number`; `utils.getLocalTimestamp()` returns GNS's process-local monotonic microsecond timestamp as a
decimal string.

`utils.getLocalPingLocation()` returns the local marker and its age. Use `parsePingLocation()`,
`estimatePingTimeBetweenLocations()`, `estimatePingTimeFromLocalHost()`, and
`checkPingDataUpToDate()` for relay-measurement workflows. `getPOPList()`, `getPingToDataCenter()`, and
`getDirectPingToPOP()` expose configured data-center measurements.

The typed `setGlobalConfigValueInt32()`, `setGlobalConfigValueFloat()`, and
`setGlobalConfigValueString()` methods, together with their `setConnection...` counterparts, expose
GNS configuration setters directly. Pass the numeric `ESteamNetworkingConfigValue` value appropriate
to the selected setter. GNS controls which values are valid at each scope and when a global value may
be set; the boolean return value is its result.

`getConfigValue(configValue, scope, scopeObject)` exposes GNS's raw configuration query. Its `value`
is the exact result `Buffer` (including a trailing NUL for a string result), and `dataType` identifies
how to interpret it. It returns `null` as `value` when GNS reports an error. `getConfigValueInfo()`
returns a configuration name, type, and permitted scope, or `null` for an unknown value.
`iterateGenericEditableConfigValues()` returns the next numeric configuration value, beginning at zero;
pass `true` as its second argument to include GNS development variables.

Set `utils.setDebugOutputLevel(level)` to queue GNS diagnostic output as `debug-output` events from
`pollEvents()`. The number is GNS's debug-output enum (0–8); level 5 (`Msg`) or 4 (`Warning`) is the
recommended normal diagnostic range.

## Connectionless messages

`messages` exposes GNS's UDP-like, identity-addressed messaging interface. Pass a valid GNS identity
string (for example, the one provided in a `messages-session-request` event), a `Buffer`, and optionally
a send flag and remote channel to `sendMessageToUser()`. Receive copied payloads from a local channel
using `receiveMessagesOnChannel()`.

Incoming sessions are reported by `pollEvents()` as `messages-session-request`; call
`acceptSessionWithUser(identityRemote)` to allow them. `messages-session-failed` reports asynchronous
connection failures. Use `closeChannelWithUser()` or `closeSessionWithUser()` when finished, and
`getSessionConnectionInfo()` for the current session state. Like the underlying standalone GNS API,
these methods require your application to provide compatible peer identities and signalling where its
chosen transport needs them.

The addon imports `@node-3d/deps-gns` before its native binary loads. That gives Windows the
dependency package's runtime search path, while Linux and macOS use the GYP rpaths into the same
`bin-*` directory. A normal `npm install` or `npm ci` therefore installs the GNS runtime without
a private SDK download.

## Binary Origin

Release archives are built by this repository's public GitHub Actions workflows.

Attestations: https://github.com/node-3d/gabenet/attestations

To verify a downloaded archive:

```bash
gh release download <tag> -R node-3d/gabenet -p <platform>.gz
gh attestation verify <platform>.gz -R node-3d/gabenet
```
