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
