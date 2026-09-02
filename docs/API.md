# API guide

`@node-3d/gabenet` is a typed ESM wrapper over the standalone
GameNetworkingSockets library. All GNS enums, flags, states, and result codes
remain numeric; use Valve's GameNetworkingSockets documentation for their
meaning.

## Lifecycle and events

Call `init()` once before using any namespace. Call `runCallbacks()` regularly,
then consume `pollEvents()` until empty. Events are copied into JavaScript and
are never delivered on a GNS worker thread. Close connections, poll groups, and
listen sockets before `shutdown()`.

`connection-status-changed` is the incoming-connection path: call
`sockets.acceptConnection(event.connection)` for a connection you want to keep.
Authentication, relay status, connectionless-message sessions, and GNS debug
output are delivered through the same event queue.

## Namespaces

- `sockets` provides connection-oriented UDP, P2P, poll-group, message, status,
  authentication, certificate, and identity operations.
- `messages` provides identity-addressed connectionless messages and session
  management.
- `utils` provides relay initialization, address/identity parsing, configuration,
  batching, timestamp, ping-location, POP, and debug-output helpers.

Connection, listen-socket, and poll-group handles are branded TypeScript
numbers. Identities and addresses are canonical GNS strings. Message numbers
and timestamps are decimal strings because their native values may exceed
JavaScript's safe integer range.

## Sending data

`sockets.sendMessageToConnection(connection, data, flags?)` is the usual
copying send path. For batching, allocate an opaque message handle with
`utils.allocateMessage(data, connection, flags?, lane?)`, then pass handles to
`sockets.sendMessages(handles, deleteFailedMessages?)`. A handle transferred to
GNS cannot be reused. With `deleteFailedMessages: false`, only failed handles
remain usable by JavaScript.

Received payloads are copied `Buffer` instances. They stay valid after the
next receive call.

## Standalone scope

The addon uses GNS's standalone interfaces. It does not load Steamworks or
require the Steam client. Ordinary IPv4/IPv6 UDP and local socket-pair workflows
need no Steam service. P2P, relay, certificate, and connectionless-message
workflows require the compatible identity, signaling, and/or relay deployment
that GNS itself expects.

The standalone public distribution does not provide FakeIP/FakeUDP, hosted SDR
server, relay-ticket parsing, or Steamworks accessor contracts. Those APIs are
therefore intentionally absent. Development coverage is tracked in
[API-CHECKLIST.md](API-CHECKLIST.md).

The default-signaling P2P entrypoints are available. Custom P2P signaling is not
yet exported while its persistent-proxy and cross-thread delivery contract is
implemented.
