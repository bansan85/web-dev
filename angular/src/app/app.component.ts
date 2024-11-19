import {
  Component,
  OnInit,
  ElementRef,
  ViewChild,
  HostListener,
} from '@angular/core';
import { WasmLoaderDemanglerService } from './wasm-loader-demangler.service';
import { WasmLoaderFormatterService } from './wasm-loader-formatter.service';
import { NgFor, NgIf, NgClass } from '@angular/common';
import { FormsModule } from '@angular/forms';
import { LucideAngularModule } from 'lucide-angular';
import { GithubMarkInlineComponent } from './img/github-mark-inline.component';
import { DialogExtComponent } from './utils/dialog-ext/dialog-ext.component';

import { EmbindModule as DemanglerModule } from '../assets/web_demangler.js';
import {
  EmbindModule as FormatterModule,
  FormatStyle,
  StringList,
} from '../assets/web_formatter.js';

@Component({
  selector: 'app-root',
  standalone: true,
  imports: [
    NgFor,
    NgIf,
    NgClass,
    FormsModule,
    LucideAngularModule,
    GithubMarkInlineComponent,
    DialogExtComponent,
  ],
  templateUrl: './app.component.html',
  styleUrl: './app.component.css',
})
export class AppComponent implements OnInit {
  demangler: DemanglerModule | undefined;
  formatter: FormatterModule | undefined;

  loadingSize: number = 0;

  demangledName: string[] = [];

  enableClangFormat: boolean = false;

  emptyStyle: FormatStyle | undefined;

  formatStyle: FormatStyle | undefined;

  isLoading: boolean = false;

  // Text by pending if text insert while wasm is loading.
  pendingText: boolean = false;

  @ViewChild('mangledInput') mangledInput!: ElementRef<HTMLTextAreaElement>;
  @ViewChild('newStyle') newStyle!: ElementRef<HTMLSelectElement>;

  @HostListener('window:resize')
  onResize() {
    this.updateIconSize();
  }

  constructor(
    private wasmLoaderDemangler: WasmLoaderDemanglerService,
    private wasmLoaderFormatter: WasmLoaderFormatterService
  ) {}

  async ngOnInit() {
    this.updateIconSize();

    await this.loadWasmDemanglerModule();

    const enableClangFormat = localStorage.getItem('enableClangFormat');
    if (enableClangFormat) {
      this.enableClangFormat = enableClangFormat === 'true';
      if (this.enableClangFormat) {
        await this.loadWasmFormatterModule();
      }
    }
  }

  async loadWasmDemanglerModule() {
    if (this.demangler || this.isLoading) {
      return;
    }
    this.isLoading = true;
    while (!this.wasmLoaderDemangler.wasm()) {
      await new Promise((resolve) => setTimeout(resolve, 100));
    }
    this.demangler = this.wasmLoaderDemangler.wasm()!;
    this.isLoading = false;
  }

  async loadWasmFormatterModule() {
    if (this.formatter || this.isLoading) {
      return;
    }
    this.isLoading = true;
    while (!this.wasmLoaderFormatter.wasm()) {
      await new Promise((resolve) => setTimeout(resolve, 100));
    }
    this.formatter = this.wasmLoaderFormatter.wasm()!;

    const formatStyleLocalStorage = localStorage.getItem('formatStyle');
    if (formatStyleLocalStorage) {
      try {
        this.formatStyle = this.formatter.deserializeFromYaml(
          formatStyleLocalStorage
        );
      } catch (error) {
        console.error(error);
        localStorage.removeItem('formatStyle');
        this.formatStyle = this.formatter.getNoStyle();
      }
    } else {
      this.formatStyle = this.formatter.getMozillaStyle();
    }
    this.emptyStyle = this.formatter.getNoStyle();

    if (this.pendingText) {
      const event = new Event('input', { bubbles: true });
      this.mangledInput.nativeElement.dispatchEvent(event);
    }
    this.pendingText = false;
    this.isLoading = false;
  }

  updateIconSize() {
    this.loadingSize = Math.min(window.innerWidth / 4, window.innerHeight / 2);
  }

  async onEnableClangFormat(value: boolean) {
    this.enableClangFormat = value;

    localStorage.setItem('enableClangFormat', value.toString());

    if (value) {
      await this.loadWasmFormatterModule();
    }

    this.reformat();
  }

  onDemangle(mangledName: string) {
    if (!this.demangler) {
      this.loadWasmDemanglerModule();
    }
    if (this.enableClangFormat) {
      this.loadWasmFormatterModule();
    }
    if (this.demangler) {
      const lines = mangledName.split('\n');
      this.demangledName = lines.map((line) =>
        this.demangler!.web_demangle(line.trim())
      );
      if (this.enableClangFormat && this.formatter && this.formatStyle) {
        this.demangledName = this.demangledName.map((line) =>
          this.formatter!.formatter(line, this.formatStyle!)
        );
      }
    } else {
      this.pendingText = true;
    }
  }

  loadStyle() {
    switch (this.newStyle.nativeElement.value) {
      case 'llvm':
        this.formatStyle = this.formatter!.getLLVMStyle();
        break;
      case 'google':
        this.formatStyle = this.formatter!.getGoogleStyle();
        break;
      case 'chromium':
        this.formatStyle = this.formatter!.getChromiumStyle();
        break;
      case 'mozilla':
        this.formatStyle = this.formatter!.getMozillaStyle();
        break;
      case 'webKit':
        this.formatStyle = this.formatter!.getWebKitStyle();
        break;
      case 'gnu':
        this.formatStyle = this.formatter!.getGNUStyle();
        break;
      case 'microsoft':
        this.formatStyle = this.formatter!.getMicrosoftStyle();
        break;
      case 'clangFormat':
        this.formatStyle = this.formatter!.getClangFormatStyle();
        break;
      case 'none':
        this.formatStyle = this.formatter!.getNoStyle();
        break;
    }

    this.reformat();
  }

  reformat() {
    const event = new Event('input', { bubbles: true });
    this.mangledInput.nativeElement.dispatchEvent(event);

    localStorage.setItem(
      'formatStyle',
      this.formatter!.serializeToYaml(this.formatStyle!)
    );
  }

  getLastStruct(root: FormatStyle, keys: (string | number)[]) {
    let target = root as any;
    for (let i = 0; i < keys.length - 1; i++) {
      if (typeof keys[i] === 'number') {
        target = target.get(keys[i]);
      } else {
        target = target[keys[i]];
      }
    }
    return target;
  }

  formatStyleKeys(keys: (string | number)[]): any {
    let target = this.getLastStruct(this.formatStyle!, keys);

    if (keys.length != 0) {
      if (typeof keys.at(-1) === 'number') {
        target = target.get(keys.at(-1));
      } else {
        target = target[keys.at(-1)!];
      }
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

  updateField(keys: (string | number)[], assign: (x: any[]) => void): void {
    let tree: any[] = [];
    let target = this.formatStyle as any;
    for (let i = 0; i < keys.length; i++) {
      if (typeof keys[i] === 'number') {
        target = target.get(keys[i]);
      } else {
        target = target[keys[i]];
      }
      tree.push(target);
    }

    assign(tree);

    for (let i = keys.length - 1; i > 0; i--) {
      if (typeof keys[i] === 'number') {
        tree[i - 1].set(keys[i], tree[i]);
        i--;
      } else {
        tree[i - 1][keys[i]] = tree[i];
      }
    }

    (this.formatStyle as any)[keys.at(0)!] = tree[0];

    this.reformat();
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

  updateRawType(
    newValue: number | boolean | string,
    keys: (string | number)[]
  ): void {
    this.updateField(keys, (x: any[]) => {
      x[x.length - 1] = newValue;
    });
  }

  isEnum(value: any): boolean {
    return typeof value === 'object' && typeof value.$$ === 'undefined';
  }

  getEnum(value: any): boolean {
    return value.constructor.name.split('_').slice(2).join('_');
  }

  allEnums(value: any): string[] {
    const items = Object.getOwnPropertyNames(
      Object.getPrototypeOf(value).constructor
    );
    return items
      .filter(
        (item) =>
          !['values', 'prototype', 'length', 'name', 'argCount'].includes(item)
      )
      .map((item) => item.split('_').slice(1).join('_'));
  }

  updateEnum(newValue: string, keys: (string | number)[]): void {
    this.updateField(keys, (x: any[]) => {
      x[x.length - 1] = (this.formatter! as any)[
        x[x.length - 1].constructor.name.split('_')[0]
      ][x[x.length - 1].constructor.name.split('_')[1] + '_' + newValue];
    });
  }

  isUndefined(value: any): boolean {
    return typeof value === 'undefined';
  }

  onUndefinedCheckboxChange(
    event: Event,
    inputValue: string,
    keys: (string | number)[]
  ) {
    this.updateField(keys, (x: any[]) => {
      const checked = (event.target as HTMLInputElement).checked;
      x[x.length - 1] = checked
        ? inputValue === ''
          ? 0
          : inputValue
        : undefined;
    });
  }

  onUndefinedInputChange(value: string, keys: (string | number)[]) {
    this.updateField(keys, (x: any[]) => {
      if (x[x.length - 1] !== undefined) {
        x[x.length - 1] = Number(value);
      }
    });
  }

  isStringList(value: any): boolean {
    return (
      typeof value === 'object' &&
      typeof value.$$ !== 'undefined' &&
      value.$$.ptrType.registeredClass.name == 'StringList'
    );
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
    this.updateField(keys, (x: any[]) => {
      (x[x.length - 1] as StringList).resize(0, '');
      const data: string = (event.target as any).value;
      data
        .split('\n')
        .forEach((data_i) => (x[x.length - 1] as StringList).push_back(data_i));
    });
  }

  isFunction(value: any): boolean {
    return typeof value === 'function';
  }

  isList(value: any): boolean {
    return (
      typeof value === 'object' &&
      typeof value.$$ !== 'undefined' &&
      typeof value.push_back === 'function' &&
      typeof value.resize === 'function' &&
      typeof value.get === 'function' &&
      typeof value.set === 'function' &&
      typeof value.size === 'function'
    );
  }

  resizeList(value: number, keys: (string | number)[]): void {
    this.updateField(keys, (x: any[]) => {
      if (x[x.length - 1].size() > value) {
        x[x.length - 1].resize(
          value,
          new (this.formatter as any)![
            x[x.length - 1].$$.ptrType.registeredClass.name.slice(0, -4)
          ]()
        );
      } else if (x[x.length - 1].size() < value) {
        for (let i = x[x.length - 1].size(); i < value; i++) {
          x[x.length - 1].push_back(
            new (this.formatter as any)![
              x[x.length - 1].$$.ptrType.registeredClass.name.slice(0, -4)
            ]()
          );
        }
      }
    });
  }

  isMiscStruct(value: any): boolean {
    return (
      typeof value === 'object' &&
      typeof value.$$ !== 'undefined' &&
      !this.isList(value)
    );
  }

  typeOf(value: any): string {
    return typeof value;
  }
}
