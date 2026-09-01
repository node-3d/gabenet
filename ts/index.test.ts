import assert from 'node:assert/strict';
import test from 'node:test';
import { init, isInitialized, shutdown } from './index.ts';

test('initializes and shuts down the standalone GNS interface', () => {
	try {
		assert.equal(isInitialized(), false);
		const result = init();
		assert.equal(result.ok, true, result.errorMessage);
		assert.equal(result.errorMessage, '');
		assert.equal(isInitialized(), true);
	} finally {
		shutdown();
	}
	assert.equal(isInitialized(), false);
});
