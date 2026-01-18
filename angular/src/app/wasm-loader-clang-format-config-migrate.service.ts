import { Injectable, signal } from '@angular/core';

import { EmbindModule as ClangFormatConfigMigrateModule } from '../assets/web_clang_format_config_migrate';
import web_clang_format_config_migrate from '../assets/web_clang_format_config_migrate.js';
import { unknownAssertError } from './apps/shared/interfaces/errors';

@Injectable({
  providedIn: 'root',
})
export class WasmLoaderClangFormatConfigMigrateService {
  private instance?: ClangFormatConfigMigrateModule;

  private readonly loading = signal(true);
  readonly isLoading = this.loading.asReadonly();

  constructor() {
    web_clang_format_config_migrate().then((instance: ClangFormatConfigMigrateModule) => {
      this.instance = instance;
      this.loading.set(false);
    }).catch((err: unknown) => {
      throw unknownAssertError(err);
    });
  }

  async wasm(): Promise<ClangFormatConfigMigrateModule> {
    if (this.isLoading()) {
      await new Promise<void>((resolve) => {
        const interval = setInterval(() => {
          if (!this.isLoading()) {
            clearInterval(interval);
            resolve();
          }
        }, 50);
      });
    }

    return this.instance!;
  }
}
