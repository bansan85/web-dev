import { defineConfig } from 'vitest/config';
import { playwright } from '@vitest/browser-playwright';

export default defineConfig({
  test: {
    globals: true,
    coverage: {
      provider: 'v8',
      reporter: ['html'],
    },
    includeTaskLocation: true,
    browser: {
      enabled: true,
      screenshotFailures: true,
      provider: playwright(),
      instances: [
        { browser: 'chromium', headless: true },
      ],
    },
  },
});
