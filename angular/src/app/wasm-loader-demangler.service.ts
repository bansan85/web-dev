import { Injectable } from '@angular/core';
import web_demangler from '../assets/web_demangler.js';

@Injectable({
  providedIn: 'root'
})
export class WasmLoaderDemanglerService {
  private instance?: any;

  constructor() {
    web_demangler().then(async (instance: any) => {
      this.instance = instance;
    });
  }

  wasm(): any {
    return this.instance;
  }
}
