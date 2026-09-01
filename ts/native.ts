import { createRequire } from 'node:module';
import { getBin } from '@node-3d/addon-tools';
import '@node-3d/deps-gns';
import '@node-3d/segfault';

export type TGabenetInitResult = Readonly<{ ok: boolean; errorMessage: string }>;

type TNative = {
	init: () => TGabenetInitResult;
	shutdown: () => void;
	isInitialized: () => boolean;
	runCallbacks: () => void;
};

const loadAddon = createRequire(import.meta.url);
export const native = loadAddon(`../${getBin()}/gabenet.node`) as TNative;
