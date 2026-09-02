import { init, pollEvents, runCallbacks, shutdown, sockets } from '@node-3d/gabenet';

const result = init();
if (!result.ok) {
	throw new Error(result.errorMessage);
}

const port = 39000 + Math.floor(Math.random() * 1000);
const listenSocket = sockets.createListenSocketIP(port, '127.0.0.1');
const client = sockets.connectByIPAddress('127.0.0.1', port);
let server: ReturnType<typeof sockets.connectByIPAddress> | null = null;

try {
	for (let attempt = 0; attempt < 100 && server === null; ++attempt) {
		runCallbacks();
		for (const event of pollEvents()) {
			if (event.type === 'connection-status-changed' && event.listenSocket === listenSocket) {
				server = event.connection;
				sockets.acceptConnection(server);
			}
		}
		// oxlint-disable-next-line eslint/no-await-in-loop -- Wait for GNS service-thread progress.
		await new Promise<void>((res) => {
			setTimeout(res, 5);
		});
	}
	if (server === null) {
		throw new Error('server did not receive a connection event');
	}

	sockets.sendMessageToConnection(client, Buffer.from('hello over UDP'));
	for (let attempt = 0; attempt < 100; ++attempt) {
		runCallbacks();
		const messages = sockets.receiveMessagesOnConnection(server);
		if (messages.length > 0) {
			console.log(messages[0].data.toString());
			break;
		}
		// oxlint-disable-next-line eslint/no-await-in-loop -- Wait for GNS service-thread progress.
		await new Promise<void>((res) => {
			setTimeout(res, 5);
		});
	}
} finally {
	sockets.closeConnection(client);
	if (server !== null) {
		sockets.closeConnection(server);
	}
	sockets.closeListenSocket(listenSocket);
	shutdown();
}
