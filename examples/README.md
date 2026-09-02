# Examples

These are repository-only consumer examples. Build the TypeScript entrypoint and
native addon first, then run either example from this package directory:

```bash
npm run build:ci
npm run build:rebuild
node examples/socket-pair.ts
node examples/udp-loopback.ts
```

Both examples import `@node-3d/gabenet` by package name and need neither
Steamworks nor a running Steam client.
