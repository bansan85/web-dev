import { Injectable } from '@angular/core';
import web_demangler from '../assets/web_demangler.js';
import { EmbindModule as DemanglerModule } from '../assets/web_demangler';

@Injectable({
  providedIn: 'root',
})
export class WasmLoaderDemanglerService {
  private instance?: DemanglerModule;

  private isLoading = true;

  constructor() {
    web_demangler().then(async (instance: DemanglerModule) => {
      this.instance = instance;
      this.isLoading = false;
    });
  }

  async wasm(): Promise<DemanglerModule> {
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
