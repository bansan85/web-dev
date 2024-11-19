import { Injectable } from '@angular/core';
import web_formatter from '../assets/web_formatter.js';
import { EmbindModule as FormatterModule } from '../assets/web_formatter';

@Injectable({
  providedIn: 'root',
})
export class WasmLoaderFormatterService {
  private instance?: FormatterModule;

  private isLoading: boolean = false;

  constructor() {}

  async wasm(): Promise<FormatterModule> {
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
    if (!this.instance) {
      this.isLoading = true;
      this.instance = await web_formatter();
      this.isLoading = false;
    }
    return this.instance!;
  }

  loading(): boolean {
    return this.isLoading;
  }
}
