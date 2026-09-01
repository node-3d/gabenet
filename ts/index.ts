import { native } from './native.ts';

export type { TGabenetInitResult } from './native.ts';

export const { init, shutdown, isInitialized, runCallbacks } = native;
