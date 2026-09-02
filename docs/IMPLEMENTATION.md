# Binding implementation contract

`gabenet` binds GameNetworkingSockets 1.6 through the standalone library
interfaces supplied by `@node-3d/deps-gns`. It never uses Steamworks interfaces.

## API rules

- Use branded numeric types for connection, listen-socket, and poll-group handles.
- Use strings for `SteamNetworkingIdentity` and `SteamNetworkingIPAddr` values.
- Use `Buffer` for binary payloads, certificates, relay tickets, and signalling data.
- Copy received GNS messages into JavaScript and release every native message before return.
- Expose GNS numeric enums and result codes as named exports.
- Validate all handles, ranges, address/identity strings, buffer lengths, and option types before
  crossing into GNS.

## Callback and lifetime rules

- GNS callbacks may come from a service thread. Native callbacks only queue plain owned data.
- JavaScript obtains queued events with `pollEvents()` after `runCallbacks()`; no JavaScript callback
  runs on a GNS thread.
- `shutdown()` clears native callback queues and invalidates all handles.
- Native objects that GNS owns are never represented as consumer-manipulable pointers.

## Standalone scope

- Bind `SteamNetworkingSockets_Lib()`, `SteamNetworkingMessages_Lib()`, and
  `SteamNetworkingUtils_Lib()` semantics only.
- Do not bind Steamworks API accessors or require a running Steam client.
- P2P custom signalling is represented by a JavaScript-owned signalling adapter, not raw C++
  `ISteamNetworkingConnectionSignaling` or receive-context pointers.
- Memory allocator, lock callbacks, and service-thread initialization hooks are process-global C++
  hooks and are not ordinary JavaScript APIs.

## Validation milestones

1. Lifecycle native-load test on all six release platforms.
2. Same-process IPv4 and IPv6 client/server loopback: listen, connect, event, accept, send,
   receive, close.
3. Cross-process client/server integration test.
4. Connectionless messages integration test.
5. Custom-signalling test with two JavaScript peers relaying opaque signalling buffers.
6. Auth, relay, and FakeIP tests only where their required credentials/services are available.
