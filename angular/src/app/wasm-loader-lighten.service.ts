import { Injectable } from '@angular/core';
import web_lighten from '../assets/web_lighten.js';
import { EmbindModule as LightenModule } from '../assets/web_lighten';

@Injectable({
  providedIn: 'root',
})
export class WasmLoaderLightenService {
  private instance?: LightenModule;

  private isLoading = true;

  constructor() {
    web_lighten().then(async (instance: LightenModule) => {
      this.instance = instance;
      this.isLoading = false;
    });
  }

  async wasm(): Promise<LightenModule> {
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
