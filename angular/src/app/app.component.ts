import { Component, OnInit, ElementRef, ViewChild } from '@angular/core';
import { WasmLoaderDemanglerService } from './wasm-loader-demangler.service';
import { WasmLoaderFormatterService } from './wasm-loader-formatter.service';
import { NgFor, NgIf, NgClass } from '@angular/common';
import { LucideAngularModule } from 'lucide-angular';

@Component({
  selector: 'app-root',
  standalone: true,
  imports: [NgFor, NgIf, NgClass, LucideAngularModule],
  templateUrl: './app.component.html',
  styleUrl: './app.component.css'
})
export class AppComponent implements OnInit {
  demangledName: string[] = [];

  isOpen: boolean = false;

  @ViewChild('dialog', { static: false }) dialogRef!: ElementRef<HTMLDialogElement>;

  constructor(private wasmLoaderDemangler: WasmLoaderDemanglerService, private wasmLoaderFormatter: WasmLoaderFormatterService) { }

  async ngOnInit() {
    if (!this.wasmLoaderDemangler.wasm()) {
      await this.loadWasmDemanglerModule();
    }
    if (!this.wasmLoaderFormatter.wasm()) {
      await this.loadWasmFormatterModule();
    }
  }

  async loadWasmDemanglerModule() {
    while (!this.wasmLoaderDemangler.wasm()) {
      await new Promise(resolve => setTimeout(resolve, 100));
    }
  }

  async loadWasmFormatterModule() {
    while (!this.wasmLoaderFormatter.wasm()) {
      await new Promise(resolve => setTimeout(resolve, 100));
    }
  }

  onDemangle(mangledName: string) {
    const wasmDemanglerInstance = this.wasmLoaderDemangler.wasm();
    const wasmFormatterInstance = this.wasmLoaderFormatter.wasm();
    if (wasmDemanglerInstance && wasmFormatterInstance && mangledName) {
      const lines = mangledName.split('\n');
      this.demangledName = lines.map(line => wasmFormatterInstance.formatter(wasmDemanglerInstance.web_demangle(line.trim())));
    }
  }

  openDialog() {
    this.dialogRef.nativeElement.showModal();
    setTimeout(() => this.isOpen = true, 0);
  }

  closeDialog() {
    const close = (event: TransitionEvent) => {
      if (event.propertyName === 'opacity') {
        this.dialogRef.nativeElement.close();
        this.dialogRef.nativeElement.removeEventListener('transitionend', close);
      }
    };

    this.dialogRef.nativeElement.addEventListener('transitionend', close);
    this.isOpen = false;
  }
}
