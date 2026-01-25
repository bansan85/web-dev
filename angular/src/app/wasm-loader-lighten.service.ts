import { Injectable, signal } from '@angular/core';

import { EmbindModule as LightenModule } from '../assets/web_lighten';
import web_lighten from '../assets/web_lighten.js';
import { unknownAssertError } from './apps/shared/interfaces/errors';

@Injectable({
  providedIn: 'root',
})
export class WasmLoaderLightenService {
  private instance?: LightenModule;

  private readonly loading = signal(true);
  readonly isLoading = this.loading.asReadonly();

  constructor() {
    web_lighten().then((instance: LightenModule) => {
      this.instance = instance;
      this.loading.set(false);
    }).catch((err: unknown) => {
      throw unknownAssertError(err);
    });
  }

  async wasm(): Promise<LightenModule> {
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
