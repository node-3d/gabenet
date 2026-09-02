import * as three from 'three';
import { addThreeHelpers, init as initCore } from '@node-3d/core';
import { init, pollEvents, runCallbacks, shutdown, sockets } from '@node-3d/gabenet';

const port = 39000;
const probeTimeoutMs = 1500;
const connectedState = 3;
const snapshotRate = 20;
const movementSpeed = 3;
const bulletSpeed = 10;
const bulletLifetimeSeconds = 1.5;
// GameNetworkingSockets' k_nSteamNetworkingSend_UnreliableNoNagle.
const unreliableNoNagle = 1;

type TConnection = ReturnType<typeof sockets.connectByIPAddress>;
type TPlayer = {
	id: number;
	x: number;
	y: number;
	up: boolean;
	down: boolean;
	left: boolean;
	right: boolean;
};
type TPosition = Readonly<Pick<TPlayer, 'id' | 'x' | 'y'>>;
type TBullet = { id: number; x: number; y: number; dx: number; dy: number; lifetime: number };
type TBulletState = Readonly<Pick<TBullet, 'id' | 'x' | 'y'>>;
type TState = Readonly<{
	type: 'state';
	you: number;
	players: readonly TPosition[];
	bullets: readonly TBulletState[];
}>;
type TInput = Readonly<{
	type: 'input';
	up: boolean;
	down: boolean;
	left: boolean;
	right: boolean;
}>;
type TShoot = Readonly<{ type: 'shoot'; targetX: number; targetY: number }>;
type TMessage = TInput | TShoot | TState;

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

const send = (connection: TConnection, value: TMessage): void => {
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

const local: TPlayer = { id: 0, x: 0, y: 0, up: false, down: false, left: false, right: false };
const players = new Map<number, TPlayer>([[local.id, local]]);
const playerTargets = new Map<number, TPosition>();
const connections = new Map<TConnection, number>();
const bullets = new Map<number, TBullet>();
let server = false;
let localPlayerId: number | null = null;
let listenSocket: ReturnType<typeof sockets.createListenSocketIP> | null = null;
let peer: TConnection | null = null;
let nextPlayerId = 1;
let nextBulletId = 1;

const spawnPosition = (id: number): Pick<TPlayer, 'x' | 'y'> => ({
	x: -6 + ((id - 1) % 4) * 4,
	y: 3 - (Math.floor((id - 1) / 4) % 2) * 6,
});

const shoot = (player: TPlayer, targetX: number, targetY: number): void => {
	const length = Math.hypot(targetX - player.x, targetY - player.y);
	if (length === 0) {
		return;
	}
	bullets.set(nextBulletId, {
		id: nextBulletId,
		x: player.x,
		y: player.y,
		dx: (targetX - player.x) / length,
		dy: (targetY - player.y) / length,
		lifetime: bulletLifetimeSeconds,
	});
	nextBulletId += 1;
};

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
			localPlayerId = local.id;
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

	const playerMeshes = new Map<number, three.Mesh>();
	const bulletMeshes = new Map<number, three.Mesh>();
	const makePlayer = (id: number): three.Mesh => {
		const mesh = new three.Mesh(
			new three.BoxGeometry(0.8, 0.8),
			new three.MeshBasicMaterial({ color: id === localPlayerId ? 0x4ade80 : 0x60a5fa }),
		);
		scene.add(mesh);
		playerMeshes.set(id, mesh);
		return mesh;
	};
	const makeBullet = (id: number): three.Mesh => {
		const mesh = new three.Mesh(
			new three.CircleGeometry(0.12, 12),
			new three.MeshBasicMaterial({ color: 0xfbbf24 }),
		);
		scene.add(mesh);
		bulletMeshes.set(id, mesh);
		return mesh;
	};

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
	doc.addEventListener('mousedown', (event) => {
		const mouseEvent = event as { button?: unknown; clientX?: unknown; clientY?: unknown };
		if (
			mouseEvent.button !== 0 ||
			typeof mouseEvent.clientX !== 'number' ||
			typeof mouseEvent.clientY !== 'number'
		) {
			return true;
		}
		const targetX = (mouseEvent.clientX / doc.innerWidth) * 16 - 8;
		const targetY = 5 - (mouseEvent.clientY / doc.innerHeight) * 10;
		if (server) {
			shoot(local, targetX, targetY);
		} else if (peer !== null) {
			send(peer, { type: 'shoot', targetX, targetY });
		}
		return true;
	});

	const syncState = (state: TState): void => {
		const isFirstState = localPlayerId === null;
		localPlayerId = state.you;
		const knownPlayers = new Set(state.players.map((player) => player.id));
		for (const player of state.players) {
			if (player.id === localPlayerId) {
				if (isFirstState) {
					local.id = player.id;
					local.x = player.x;
					local.y = player.y;
					players.delete(0);
					players.set(player.id, local);
				}
				continue;
			}
			const remote = players.get(player.id);
			if (remote === undefined) {
				players.set(player.id, {
					...player,
					up: false,
					down: false,
					left: false,
					right: false,
				});
			}
			playerTargets.set(player.id, player);
		}
		for (const [id] of players) {
			if (id !== localPlayerId && !knownPlayers.has(id)) {
				players.delete(id);
			}
		}
		const knownBullets = new Set(state.bullets.map((bullet) => bullet.id));
		for (const bullet of state.bullets) {
			const existing = bullets.get(bullet.id);
			if (existing === undefined) {
				bullets.set(bullet.id, { ...bullet, dx: bullet.x, dy: bullet.y, lifetime: 0 });
			} else {
				existing.dx = bullet.x;
				existing.dy = bullet.y;
			}
		}
		for (const [id] of bullets) {
			if (!knownBullets.has(id)) {
				bullets.delete(id);
			}
		}
	};

	let lastFrame = Date.now();
	let lastSnapshot = 0;
	const acceptConnections = (): void => {
		for (const event of pollEvents()) {
			if (
				server &&
				event.type === 'connection-status-changed' &&
				event.listenSocket === listenSocket &&
				!connections.has(event.connection)
			) {
				sockets.acceptConnection(event.connection);
				const id = nextPlayerId;
				nextPlayerId += 1;
				players.set(id, {
					id,
					...spawnPosition(id),
					up: false,
					down: false,
					left: false,
					right: false,
				});
				connections.set(event.connection, id);
			}
		}
	};
	const receiveMessages = (): void => {
		if (server) {
			for (const [connection, playerId] of connections) {
				for (const message of sockets.receiveMessagesOnConnection(connection)) {
					const value = JSON.parse(message.data.toString()) as TInput | TShoot;
					const player = players.get(playerId);
					if (player === undefined) {
						continue;
					}
					if (value.type === 'input') {
						Object.assign(player, value);
					}
					if (value.type === 'shoot') {
						shoot(player, value.targetX, value.targetY);
					}
				}
			}
		} else if (peer !== null) {
			for (const message of sockets.receiveMessagesOnConnection(peer)) {
				const value = JSON.parse(message.data.toString()) as TState;
				if (value.type === 'state') {
					syncState(value);
				}
			}
		}
	};
	const updateSimulation = (seconds: number): void => {
		move(local, seconds);
		if (server) {
			for (const player of players.values()) {
				if (player !== local) {
					move(player, seconds);
				}
			}
			for (const [id, bullet] of bullets) {
				bullet.x += bullet.dx * bulletSpeed * seconds;
				bullet.y += bullet.dy * bulletSpeed * seconds;
				bullet.lifetime -= seconds;
				if (bullet.lifetime <= 0) {
					bullets.delete(id);
				}
			}
		} else {
			const interpolation = Math.min(seconds * 12, 1);
			for (const [id, player] of players) {
				const target = playerTargets.get(id);
				if (id === localPlayerId || target === undefined) {
					continue;
				}
				player.x += (target.x - player.x) * interpolation;
				player.y += (target.y - player.y) * interpolation;
			}
			for (const bullet of bullets.values()) {
				bullet.x += (bullet.dx - bullet.x) * interpolation;
				bullet.y += (bullet.dy - bullet.y) * interpolation;
			}
		}
	};
	const sendSnapshot = (now: number): void => {
		if (now - lastSnapshot >= 1000 / snapshotRate) {
			if (server) {
				const snapshot = (you: number): TState => ({
					type: 'state',
					you,
					players: [...players.values()].map(({ id, x, y }) => ({ id, x, y })),
					bullets: [...bullets.values()].map(({ id, x, y }) => ({ id, x, y })),
				});
				for (const [connection, playerId] of connections) {
					send(connection, snapshot(playerId));
				}
			} else if (peer !== null) {
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
	};
	const renderScene = (): void => {
		for (const [id, player] of players) {
			const mesh = playerMeshes.get(id) ?? makePlayer(id);
			mesh.position.set(player.x, player.y, 0);
		}
		for (const [id, mesh] of playerMeshes) {
			if (!players.has(id)) {
				scene.remove(mesh);
				playerMeshes.delete(id);
			}
		}
		for (const [id, bullet] of bullets) {
			const mesh = bulletMeshes.get(id) ?? makeBullet(id);
			mesh.position.set(bullet.x, bullet.y, 0);
		}
		for (const [id, mesh] of bulletMeshes) {
			if (!bullets.has(id)) {
				scene.remove(mesh);
				bulletMeshes.delete(id);
			}
		}
		renderer.render(scene, camera);
	};
	const frame = (): void => {
		const now = Date.now();
		const seconds = Math.min((now - lastFrame) / 1000, 0.05);
		lastFrame = now;
		runCallbacks();
		acceptConnections();
		receiveMessages();
		updateSimulation(seconds);
		sendSnapshot(now);
		renderScene();
		raf(frame);
	};
	raf(frame);
} catch (error) {
	shutdown();
	throw error;
}
