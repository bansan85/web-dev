import { Injectable, signal } from '@angular/core';

import { EmbindModule as DemanglerModule } from '../assets/web_demangler';
import web_demangler from '../assets/web_demangler.js';
import { unknownAssertError } from './apps/shared/interfaces/errors';

export type { EmbindModule as DemanglerModule } from '../assets/web_demangler';

@Injectable({
  providedIn: 'root',
})
export class WasmLoaderDemanglerService {
  private instance?: DemanglerModule;

  private readonly loading = signal(true);
  readonly isLoading = this.loading.asReadonly();

  constructor() {
    web_demangler().then((instance: DemanglerModule) => {
      this.instance = instance;
      this.loading.set(false);
    }).catch((err: unknown) => {
      throw unknownAssertError(err);
    });
  }

  async wasm(): Promise<DemanglerModule> {
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
