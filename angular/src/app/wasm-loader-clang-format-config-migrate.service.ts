import { Injectable } from '@angular/core';
import web_clang_format_config_migrate from '../assets/web_clang_format_config_migrate.js';
import { EmbindModule as ClangFormatConfigMigrateModule } from '../assets/web_clang_format_config_migrate';

@Injectable({
  providedIn: 'root',
})
export class WasmLoaderClangFormatConfigMigrateService {
  private instance?: ClangFormatConfigMigrateModule;

  private isLoading = true;

  constructor() {
    web_clang_format_config_migrate().then(async (instance: ClangFormatConfigMigrateModule) => {
      this.instance = instance;
      this.isLoading = false;
    });
  }

  async wasm(): Promise<ClangFormatConfigMigrateModule> {
    if (this.isLoading) {
      await new Promise<void>((resolve) => {
        const interval = setInterval(() => {
          if (!this.isLoading) {
            clearInterval(interval);
            resolve();
          }
        }, 50);
      });
    }

    return this.instance!;
  }

  loading(): boolean {
    return this.isLoading;
  }
}
