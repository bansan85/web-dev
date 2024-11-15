import { Injectable } from '@angular/core';
import web_formatter from '../assets/web_formatter.js';
import { EmbindModule as FormatterModule } from '../assets/web_formatter';

@Injectable({
  providedIn: 'root',
})
export class WasmLoaderFormatterService {
  private instance?: FormatterModule;

  constructor() {}

  wasm(): FormatterModule | undefined {
    if (!this.instance) {
      web_formatter().then(async (instance: FormatterModule) => {
        this.instance = instance;
      });
    }
    return this.instance;
  }
}
