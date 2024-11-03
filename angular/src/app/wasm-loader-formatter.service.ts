import { Injectable } from '@angular/core';
import web_formatter from '../assets/web_formatter.js';
import { EmbindModule as FormatterModule } from '../assets/web_formatter.js';

@Injectable({
  providedIn: 'root'
})
export class WasmLoaderFormatterService {
  private instance?: FormatterModule;

  constructor() {
    web_formatter().then(async (instance: FormatterModule) => {
      this.instance = instance;
    });
  }

  wasm(): FormatterModule | undefined {
    return this.instance;
  }
}
