import { Component, OnInit, ElementRef, ViewChild } from '@angular/core';
import { WasmLoaderDemanglerService } from './wasm-loader-demangler.service';
import { WasmLoaderFormatterService } from './wasm-loader-formatter.service';
import { NgFor, NgIf, NgClass } from '@angular/common';
import { FormsModule } from '@angular/forms';
import { LucideAngularModule } from 'lucide-angular';

import { EmbindModule as DemanglerModule } from '../assets/web_demangler.js';
import { EmbindModule as FormatterModule, FormatStyle, StringList, RawStringFormat } from '../assets/web_formatter.js';

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

  emptyStyle: FormatStyle | undefined;

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
    this.emptyStyle = this.formatter.getNoStyle();

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

  reformat() {
    const event = new Event('input', { bubbles: true });
    this.mangledInput.nativeElement.dispatchEvent(event);
  }

  getLastStruct(root: FormatStyle, keys: (string | number)[]) {
    let target = root as any;
    for (let i = 0; i < keys.length - 1; i++) {
      if (typeof keys[i] === "number") {
        target = target.get(keys[i]);
      } else {
        target = target[keys[i]];
      }
    }
    return target;
  }

  formatStyleKeys(keys: (string | number)[]): any {
    let target = this.getLastStruct(this.emptyStyle!, keys);
    if (keys.length != 0) {
      target = target[keys.at(-1)!];
    }

    const retval: any = [];

    if (!this.formatter) {
      return retval;
    }

    for (const key in target) {
      if (target.hasOwnProperty(key) || key in target) {
        retval.push(key);
      }
    }

    return retval;
  }

  isNumber(value: any): boolean {
    return typeof value === 'number';
  }

  isBoolean(value: any): boolean {
    return typeof value === 'boolean';
  }

  isString(value: any): boolean {
    return typeof value === 'string';
  }

  isEnum(value: any): boolean {
    return typeof value === 'object' && typeof value.$$ === 'undefined';
  }

  getEnum(value: any): boolean {
    return value.constructor.name.split('_').slice(2).join('_');
  }

  allEnums(value: any): string[] {
    const items = Object.getOwnPropertyNames(Object.getPrototypeOf(value).constructor);
    return items.filter(item => !["values", "prototype", "length", "name"].includes(item)).map(item => item.split('_').slice(1).join('_'));
  }

  updateEnum(newValue: string, keys: (string | number)[]): void {
    const target = this.getLastStruct(this.formatStyle!, keys);
    const lastKey = keys.at(-1)!;

    target[lastKey] = (this.formatter! as any)[target[lastKey].constructor.name.split('_')[0]][target[lastKey].constructor.name.split('_')[1] + '_' + newValue]
    this.reformat();
  }

  isUndefined(value: any): boolean {
    return typeof value === 'undefined';
  }

  onUndefinedCheckboxChange(event: Event, inputValue: string, keys: (string | number)[]) {
    const target = this.getLastStruct(this.formatStyle!, keys);
    const lastKey = keys.at(-1)!;

    const checked = (event.target as HTMLInputElement).checked;
    target[lastKey] = checked ? inputValue : undefined;
  }

  onUndefinedInputChange(value: string, keys: (string | number)[]) {
    const target = this.getLastStruct(this.formatStyle!, keys);
    const lastKey = keys.at(-1)!;

    if (target[lastKey] !== undefined) {
      target[lastKey] = value;
    }
  }

  isStringList(value: any): boolean {
    return typeof value === 'object' && typeof value.$$ !== 'undefined' && value.$$.ptrType.registeredClass.name == 'StringList';
  }

  stringListToTextArea(raw_value: any): string {
    let value: StringList = raw_value as StringList;
    let retval: string[] = [];
    for (let i = 0; i < value.size(); i++) {
      retval.push(value.get(i) as string);
    }
    return retval.join('\n');
  }

  public onStringList(event: Event, keys: (string | number)[]): void {
    const target = this.getLastStruct(this.formatStyle!, keys);
    const lastKey = keys.at(-1)!;

    (target[lastKey] as StringList).resize(0, '');
    const data: string = (event.target as any).value;
    data.split('\n').forEach(data_i => (target[lastKey] as StringList).push_back(data_i));

    this.reformat();
  }

  isFunction(value: any): boolean {
    return typeof value === 'function';
  }

  isMiscStruct(value: any): boolean {
    return typeof value === 'object' && typeof value.$$ !== 'undefined' && !this.isStringList(value);
  }

  typeOf(value: any): string {
    return typeof value;
  }
}
