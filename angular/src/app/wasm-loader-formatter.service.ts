import { Injectable, signal } from '@angular/core';

import { EmbindModule as FormatterModule } from '../assets/web_formatter';
import web_formatter from '../assets/web_formatter.js';

export type {
  EmbindModule as FormatterModule,
  FormatStyle,
  StringList,
} from '../assets/web_formatter';

@Injectable({
  providedIn: 'root',
})
export class WasmLoaderFormatterService {
  private instance?: FormatterModule;

  private readonly loading = signal(false);
  readonly isLoading = this.loading.asReadonly();

  async wasm(): Promise<FormatterModule> {
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
    if (!this.instance) {
      this.loading.set(true);
      this.instance = await web_formatter();
      this.loading.set(false);
    }
    return this.instance;
  }
}
