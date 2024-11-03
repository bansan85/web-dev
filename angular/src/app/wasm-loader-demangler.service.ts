import { Injectable } from '@angular/core';
import web_demangler from '../assets/web_demangler.js';
import { EmbindModule as DemanglerModule } from '../assets/web_demangler.js';

@Injectable({
  providedIn: 'root'
})
export class WasmLoaderDemanglerService {
  private instance?: DemanglerModule;

  constructor() {
    web_demangler().then(async (instance: DemanglerModule) => {
      this.instance = instance;
    });
  }

  wasm(): DemanglerModule | undefined {
    return this.instance;
  }
}
