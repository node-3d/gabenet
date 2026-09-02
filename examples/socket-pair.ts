import { init, runCallbacks, shutdown, sockets, utils } from '@node-3d/gabenet';

const result = init();
if (!result.ok) {
	throw new Error(result.errorMessage);
}

const pair = sockets.createSocketPair();
if (pair === null) {
	throw new Error('GNS did not create a local socket pair');
}

const [sender, receiver] = pair;
try {
	const message = utils.allocateMessage(Buffer.from('hello from gabenet'), sender);
	if (message === null) {
		throw new Error('GNS did not allocate a message');
	}
	sockets.sendMessages([message]);

	for (let attempt = 0; attempt < 100; ++attempt) {
		runCallbacks();
		const messages = sockets.receiveMessagesOnConnection(receiver);
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
	sockets.closeConnection(sender);
	sockets.closeConnection(receiver);
	shutdown();
}
