import { Injectable } from '@angular/core';
import web_formatter from '../assets/web_formatter.js';

@Injectable({
  providedIn: 'root'
})
export class WasmLoaderFormatterService {
  private instance?: any;

  constructor() {
    web_formatter().then(async (instance: any) => {
      this.instance = instance;
    });
  }

  wasm(): any {
    return this.instance;
  }
}
