import * as three from 'three';
import { addThreeHelpers, init as initCore } from '@node-3d/core';
import { init, pollEvents, runCallbacks, shutdown, sockets } from '@node-3d/gabenet';

const port = 39000;
const probeTimeoutMs = 1500;
const connectedState = 3;
const snapshotRate = 20;
const movementSpeed = 3;
// GameNetworkingSockets' k_nSteamNetworkingSend_UnreliableNoNagle.
const unreliableNoNagle = 1;

type TPlayer = { x: number; y: number; up: boolean; down: boolean; left: boolean; right: boolean };
type TState = Readonly<{
	type: 'state';
	server: Pick<TPlayer, 'x' | 'y'>;
	client: Pick<TPlayer, 'x' | 'y'>;
}>;
type TInput = Readonly<{
	type: 'input';
	up: boolean;
	down: boolean;
	left: boolean;
	right: boolean;
}>;

const wait = (milliseconds: number): Promise<void> =>
	new Promise((res) => {
		setTimeout(res, milliseconds);
	});

const move = (player: TPlayer, seconds: number): void => {
	player.x += (Number(player.right) - Number(player.left)) * movementSpeed * seconds;
	player.y += (Number(player.up) - Number(player.down)) * movementSpeed * seconds;
	player.x = Math.max(-7.5, Math.min(7.5, player.x));
	player.y = Math.max(-4.5, Math.min(4.5, player.y));
};

const send = (
	connection: ReturnType<typeof sockets.connectByIPAddress>,
	value: TInput | TState,
): void => {
	sockets.sendMessageToConnection(
		connection,
		Buffer.from(JSON.stringify(value)),
		unreliableNoNagle,
	);
};

const result = init();
if (!result.ok) {
	throw new Error(result.errorMessage);
}

const local: TPlayer = { x: 0, y: 0, up: false, down: false, left: false, right: false };
const remote: TPlayer = { x: 0, y: 0, up: false, down: false, left: false, right: false };
const remoteTarget = { x: 0, y: 0 };
let server = false;
let listenSocket: ReturnType<typeof sockets.createListenSocketIP> | null = null;
let peer: ReturnType<typeof sockets.connectByIPAddress> | null = null;

try {
	const probe = sockets.connectByIPAddress('127.0.0.1', port);
	const deadline = Date.now() + probeTimeoutMs;
	while (Date.now() < deadline && peer === null) {
		runCallbacks();
		for (const event of pollEvents()) {
			if (
				event.type === 'connection-status-changed' &&
				event.connection === probe &&
				event.state === connectedState
			) {
				peer = probe;
			}
		}
		// oxlint-disable-next-line eslint/no-await-in-loop -- Waiting for network progress.
		await wait(10);
	}

	if (peer === null) {
		sockets.closeConnection(probe);
		listenSocket = sockets.createListenSocketIP(port, '127.0.0.1');
		if (listenSocket === 0) {
			peer = sockets.connectByIPAddress('127.0.0.1', port);
		} else {
			server = true;
		}
	}

	const { doc, raf } = initCore({
		autoEsc: true,
		title: server ? 'Gabenet UDP server' : 'Gabenet UDP client',
	});
	addThreeHelpers(three);
	const scene = new three.Scene();
	const camera = new three.OrthographicCamera(-8, 8, 5, -5, 0, 10);
	camera.position.z = 1;
	const renderer = new three.WebGLRenderer();
	renderer.setSize(doc.innerWidth, doc.innerHeight);
	doc.body.appendChild(renderer.domElement);

	const makePlayer = (color: number): three.Mesh => {
		const mesh = new three.Mesh(
			new three.BoxGeometry(0.8, 0.8),
			new three.MeshBasicMaterial({ color }),
		);
		scene.add(mesh);
		return mesh;
	};
	const localMesh = makePlayer(server ? 0x4ade80 : 0x60a5fa);
	const remoteMesh = makePlayer(server ? 0x60a5fa : 0x4ade80);

	const updateKeys = (value: boolean, key: string): void => {
		if (key === 'ArrowUp' || key === 'w') {
			local.up = value;
		}
		if (key === 'ArrowDown' || key === 's') {
			local.down = value;
		}
		if (key === 'ArrowLeft' || key === 'a') {
			local.left = value;
		}
		if (key === 'ArrowRight' || key === 'd') {
			local.right = value;
		}
	};
	doc.addEventListener('keydown', (event) => {
		if (typeof event.key === 'string') {
			updateKeys(true, event.key);
		}
		return true;
	});
	doc.addEventListener('keyup', (event) => {
		if (typeof event.key === 'string') {
			updateKeys(false, event.key);
		}
		return true;
	});

	let lastFrame = Date.now();
	let lastSnapshot = 0;
	const frame = (): void => {
		const now = Date.now();
		const seconds = Math.min((now - lastFrame) / 1000, 0.05);
		lastFrame = now;
		runCallbacks();
		for (const event of pollEvents()) {
			if (
				server &&
				event.type === 'connection-status-changed' &&
				event.listenSocket === listenSocket
			) {
				sockets.acceptConnection(event.connection);
				peer = event.connection;
			}
		}
		if (peer !== null) {
			for (const message of sockets.receiveMessagesOnConnection(peer)) {
				const value = JSON.parse(message.data.toString()) as TInput | TState;
				if (server && value.type === 'input') {
					Object.assign(remote, value);
				}
				if (!server && value.type === 'state') {
					remoteTarget.x = value.server.x;
					remoteTarget.y = value.server.y;
				}
			}
		}

		move(local, seconds);
		if (server) {
			move(remote, seconds);
		} else {
			const interpolation = Math.min(seconds * 12, 1);
			remote.x += (remoteTarget.x - remote.x) * interpolation;
			remote.y += (remoteTarget.y - remote.y) * interpolation;
		}
		if (peer !== null && now - lastSnapshot >= 1000 / snapshotRate) {
			if (server) {
				send(peer, { type: 'state', server: local, client: remote });
			} else {
				send(peer, {
					type: 'input',
					up: local.up,
					down: local.down,
					left: local.left,
					right: local.right,
				});
			}
			lastSnapshot = now;
		}
		localMesh.position.set(local.x, local.y, 0);
		remoteMesh.position.set(remote.x, remote.y, 0);
		renderer.render(scene, camera);
		raf(frame);
	};
	raf(frame);
} catch (error) {
	shutdown();
	throw error;
}
