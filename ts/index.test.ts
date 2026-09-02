import assert from 'node:assert/strict';
import test from 'node:test';
import type { TConnection } from './index.ts';
import {
	init,
	isInitialized,
	messages,
	pollEvents,
	runCallbacks,
	shutdown,
	sockets,
	utils,
} from './index.ts';

const delay = (): Promise<void> =>
	new Promise((res) => {
		setTimeout(res, 5);
	});

test('initializes and shuts down the standalone GNS interface', () => {
	try {
		assert.equal(isInitialized(), false);
		const result = init();
		assert.equal(result.ok, true, result.errorMessage);
		assert.equal(result.errorMessage, '');
		assert.equal(isInitialized(), true);
		assert.equal(typeof sockets.getAuthenticationStatus().availability, 'number');
		assert.equal(typeof utils.getRelayNetworkStatus().availability, 'number');
		assert.equal(utils.parseIPAddress('127.0.0.1:30000'), '127.0.0.1:30000');
		assert.equal(utils.parseIPAddress('not an address'), null);
		assert.equal(utils.parseIdentity('not an identity'), null);
		assert.equal(typeof utils.getIPv4FakeIPType(0), 'number');
		assert.equal(typeof utils.isFakeIPv4(0), 'boolean');
		assert.match(utils.getLocalTimestamp(), /^\d+$/u);
		assert.equal(typeof utils.setGlobalConfigValueInt32, 'function');
		assert.equal(typeof utils.setGlobalConfigValueFloat, 'function');
		assert.equal(typeof utils.setGlobalConfigValueString, 'function');
		assert.equal(typeof utils.setConnectionConfigValueInt32, 'function');
		assert.equal(typeof utils.setConnectionConfigValueFloat, 'function');
		assert.equal(typeof utils.setConnectionConfigValueString, 'function');
		const configValue = utils.iterateGenericEditableConfigValues();
		assert.notEqual(configValue, 0);
		assert.notEqual(utils.getConfigValueInfo(configValue), null);
		const config = utils.getConfigValue(configValue, 1, 0);
		assert.ok(config.result > 0);
		assert.ok(Buffer.isBuffer(config.value));
		utils.setDebugOutputLevel(0);
	} finally {
		shutdown();
	}
	assert.equal(isInitialized(), false);
});

test('exchanges a message through a local socket pair and configures lanes', async () => {
	let connection1: TConnection | undefined = undefined;
	let connection2: TConnection | undefined = undefined;

	try {
		assert.equal(init().ok, true);
		const pair = sockets.createSocketPair();
		if (pair === null) {
			throw new Error('GNS did not create a local socket pair');
		}
		const [firstConnection, secondConnection] = pair;
		connection1 = firstConnection;
		connection2 = secondConnection;
		assert.equal(
			sockets.configureConnectionLanes(
				firstConnection,
				2,
				new Int32Array([0, 1]),
				new Uint16Array([1, 1]),
			),
			1,
		);
		const message = utils.allocateMessage(Buffer.from('socket pair'), firstConnection);
		if (message === null) {
			throw new Error('GNS did not allocate a message');
		}
		assert.equal(sockets.sendMessages([message]).length, 1);

		let received: ReturnType<typeof sockets.receiveMessagesOnConnection> = [];
		for (let attempt = 0; attempt < 100 && received.length === 0; ++attempt) {
			runCallbacks();
			received = sockets.receiveMessagesOnConnection(secondConnection);
			// oxlint-disable-next-line eslint/no-await-in-loop -- Wait for GNS service-thread progress.
			await delay();
		}
		assert.equal(received.length, 1);
		assert.equal(received[0].data.toString(), 'socket pair');
	} finally {
		if (connection1) {
			sockets.closeConnection(connection1);
		}
		if (connection2) {
			sockets.closeConnection(connection2);
		}
		shutdown();
	}
});

test('rejects invalid connectionless message identities', () => {
	try {
		assert.equal(init().ok, true);
		assert.throws(() => messages.acceptSessionWithUser('not a GNS identity'));
		assert.throws(() => sockets.connectP2P('not a GNS identity'));
	} finally {
		shutdown();
	}
});

test('exchanges a reliable message through a local UDP listen socket', async () => {
	const port = 39123;
	let listenSocket: ReturnType<typeof sockets.createListenSocketIP> | undefined = undefined;
	let client: ReturnType<typeof sockets.connectByIPAddress> | undefined = undefined;
	let server: ReturnType<typeof sockets.connectByIPAddress> | undefined = undefined;
	let pollGroup: ReturnType<typeof sockets.createPollGroup> | undefined = undefined;

	try {
		assert.equal(init().ok, true);
		listenSocket = sockets.createListenSocketIP(port, '127.0.0.1');
		client = sockets.connectByIPAddress('127.0.0.1', port);

		for (let attempt = 0; attempt < 100 && !server; ++attempt) {
			runCallbacks();
			for (const event of pollEvents()) {
				if (
					event.type === 'connection-status-changed' &&
					event.listenSocket === listenSocket
				) {
					server = event.connection;
					assert.equal(sockets.acceptConnection(event.connection), 1);
				}
			}
			// oxlint-disable-next-line eslint/no-await-in-loop -- Wait for GNS service-thread UDP progress.
			await delay();
		}

		if (server === undefined) {
			throw new Error('server did not receive a connection event');
		}
		const serverConnection = server;
		sockets.setConnectionName(serverConnection, 'local test server');
		assert.equal(sockets.getConnectionName(serverConnection), 'local test server');
		assert.equal(sockets.getConnectionInfo(serverConnection)?.connection, serverConnection);
		assert.equal(sockets.setConnectionUserData(serverConnection, 123), true);
		assert.equal(sockets.getConnectionUserData(serverConnection), 123);
		assert.equal(sockets.getConnectionRealTimeStatus(serverConnection).result, 1);
		assert.equal(typeof sockets.getDetailedConnectionStatus(serverConnection), 'string');
		assert.ok(sockets.getIdentity() === null || typeof sockets.getIdentity() === 'string');
		pollGroup = sockets.createPollGroup();
		assert.equal(sockets.setConnectionPollGroup(serverConnection, pollGroup), true);
		sockets.sendMessageToConnection(client, Buffer.from('hello from gabenet'));

		let messages: ReturnType<typeof sockets.receiveMessagesOnPollGroup> = [];
		for (let attempt = 0; attempt < 100 && messages.length === 0; ++attempt) {
			runCallbacks();
			messages = sockets.receiveMessagesOnPollGroup(pollGroup);
			// oxlint-disable-next-line eslint/no-await-in-loop -- Wait for GNS service-thread UDP progress.
			await delay();
		}

		assert.equal(messages.length, 1);
		assert.equal(messages[0].data.toString(), 'hello from gabenet');
	} finally {
		if (client) {
			sockets.closeConnection(client);
		}
		if (server) {
			sockets.closeConnection(server);
		}
		if (listenSocket) {
			sockets.closeListenSocket(listenSocket);
		}
		if (pollGroup) {
			sockets.destroyPollGroup(pollGroup);
		}
		shutdown();
	}
});
