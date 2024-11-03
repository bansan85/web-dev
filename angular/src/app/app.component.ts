import { Component, OnInit, ElementRef, ViewChild } from '@angular/core';
import { WasmLoaderDemanglerService } from './wasm-loader-demangler.service';
import { WasmLoaderFormatterService } from './wasm-loader-formatter.service';
import { NgFor, NgIf, NgClass } from '@angular/common';
import { FormsModule } from '@angular/forms';
import { LucideAngularModule } from 'lucide-angular';

import { EmbindModule as DemanglerModule } from '../assets/web_demangler.js';
import { EmbindModule as FormatterModule, FormatStyle } from '../assets/web_formatter.js';

@Component({
  selector: 'app-root',
  standalone: true,
  imports: [NgFor, NgIf, NgClass, FormsModule, LucideAngularModule],
  templateUrl: './app.component.html',
  styleUrl: './app.component.css'
})
export class AppComponent implements OnInit {
  demangler: DemanglerModule | undefined;
  formatter: FormatterModule | undefined;

  demangledName: string[] = [];

  isOpen: boolean = false;

  formatStyle: FormatStyle | undefined;

  // Text by pending if text insert while wasm is loading.
  pendingText: boolean = false;

  @ViewChild('dialog', { static: false }) dialogRef!: ElementRef<HTMLDialogElement>;
  @ViewChild('mangledInput') mangledInput!: ElementRef<HTMLTextAreaElement>;

  constructor(private wasmLoaderDemangler: WasmLoaderDemanglerService,
    private wasmLoaderFormatter: WasmLoaderFormatterService) { }

  async ngOnInit() {
    if (!this.demangler) {
      await this.loadWasmDemanglerModule();
    }
    if (!this.formatter) {
      await this.loadWasmFormatterModule();
    }
  }

  async loadWasmDemanglerModule() {
    while (!this.wasmLoaderDemangler.wasm()) {
      await new Promise(resolve => setTimeout(resolve, 100));
    }
    this.demangler = this.wasmLoaderDemangler.wasm()!;
  }

  async loadWasmFormatterModule() {
    while (!this.wasmLoaderFormatter.wasm()) {
      await new Promise(resolve => setTimeout(resolve, 100));
    }
    this.formatter = this.wasmLoaderFormatter.wasm()!;

    this.formatStyle = this.formatter.getMozillaStyle();

    if (this.pendingText) {
      const event = new Event('input', { bubbles: true });
      this.mangledInput.nativeElement.dispatchEvent(event);
    }
    this.pendingText = false;
  }

  onDemangle(mangledName: string) {
    if (!this.demangler) {
      this.loadWasmDemanglerModule();
    }
    if (!this.formatter) {
      this.loadWasmFormatterModule();
    }
    if (this.demangler && this.formatter && this.formatStyle) {
      const lines = mangledName.split('\n');
      this.demangledName = lines.map(line =>
        this.formatter!.formatter(
          this.demangler!.web_demangle(line.trim()), this.formatStyle!));
    } else {
      this.pendingText = true;
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

  onColumnLimitChange(newValue: number) {
    if (this.formatStyle) {
      this.formatStyle.ColumnLimit = newValue;
      const event = new Event('input', { bubbles: true });
      this.mangledInput.nativeElement.dispatchEvent(event);
    }
  }
}
