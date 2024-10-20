import { Component, OnInit } from '@angular/core';
import { WasmLoaderService } from './wasm-loader.service';
import { NgFor, NgIf } from '@angular/common';

@Component({
  selector: 'app-root',
  standalone: true,
  imports: [NgFor, NgIf],
  templateUrl: './app.component.html',
  styleUrl: './app.component.css'
})
export class AppComponent implements OnInit {
  demangledName: string[] = [];

  constructor(private wasmLoader: WasmLoaderService) { }

  async ngOnInit() {
    if (!this.wasmLoader.wasm()) {
      await this.loadWasmModule();
    }
  }

  async loadWasmModule() {
    while (!this.wasmLoader.wasm()) {
      await new Promise(resolve => setTimeout(resolve, 100));
    }
  }

  onDemangle(mangledName: string) {
    const wasmInstance = this.wasmLoader.wasm();
    if (wasmInstance && mangledName) {
      const lines = mangledName.split('\n');
      this.demangledName = lines.map(line => wasmInstance.demangle(line.trim()));
    }
  }
}
