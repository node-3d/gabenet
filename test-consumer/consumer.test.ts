import assert from 'node:assert/strict';
import test from 'node:test';
import { init, isInitialized, shutdown } from '@node-3d/gabenet';

test('loads the packed GameNetworkingSockets addon', () => {
	assert.equal(typeof init, 'function');
	assert.equal(typeof isInitialized, 'function');
	assert.equal(typeof shutdown, 'function');
});
