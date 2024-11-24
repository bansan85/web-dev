import {
  Component,
  OnInit,
  ElementRef,
  ViewChild,
  HostListener,
  ChangeDetectorRef,
  ViewEncapsulation,
} from '@angular/core';
import { WasmLoaderDemanglerService } from './wasm-loader-demangler.service';
import { WasmLoaderFormatterService } from './wasm-loader-formatter.service';
import { NgIf } from '@angular/common';
import { FormsModule } from '@angular/forms';
import { LucideAngularModule } from 'lucide-angular';
import { GithubMarkInlineComponent } from './img/github-mark-inline.component';
import { DialogExtComponent } from './utils/dialog-ext/dialog-ext.component';
import { FormatterOptionsComponent } from './formatter-options/formatter-options.component';

import { EmbindModule as DemanglerModule } from '../assets/web_demangler.js';
import {
  EmbindModule as FormatterModule,
  FormatStyle,
} from '../assets/web_formatter.js';

@Component({
  selector: 'app-root',
  standalone: true,
  imports: [
    NgIf,
    FormsModule,
    LucideAngularModule,
    GithubMarkInlineComponent,
    DialogExtComponent,
    FormatterOptionsComponent,
  ],
  templateUrl: './app.component.html',
  styleUrl: './app.component.css',
  encapsulation: ViewEncapsulation.None,
})
export class AppComponent implements OnInit {
  demangler?: DemanglerModule;
  formatter?: FormatterModule;

  loadingSize = 0;

  demangledName: string[] = [];

  enableClangFormat = false;
  enableClangFormatExpert = false;

  formatStyle?: FormatStyle;
  emptyStyle?: FormatStyle;

  // Text by pending if text insert while wasm is loading.
  pendingText = false;

  @ViewChild('mangledInput') mangledInput!: ElementRef<HTMLTextAreaElement>;
  @ViewChild('newStyle') newStyle!: ElementRef<HTMLSelectElement>;
  @ViewChild('dialog') dialog!: DialogExtComponent;
  @ViewChild('textClangConfig')
  textClangConfig!: ElementRef<HTMLTextAreaElement>;

  @HostListener('window:resize')
  onResize() {
    this.updateIconSize();
  }

  constructor(
    private wasmLoaderDemangler: WasmLoaderDemanglerService,
    private wasmLoaderFormatter: WasmLoaderFormatterService,
    private cdr: ChangeDetectorRef
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

    this.enableClangFormatExpert =
      localStorage.getItem('enableClangFormatExpert') === 'true';
  }

  async loadWasmDemanglerModule() {
    if (!this.demangler) {
      this.demangler = await this.wasmLoaderDemangler.wasm();
    }
  }

  async loadWasmFormatterModule() {
    if (this.formatter) {
      return;
    }
    this.formatter = await this.wasmLoaderFormatter.wasm();

    if (this.formatStyle) {
      return;
    }

    // No await after this comment.
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

    if (value && this.enableClangFormatExpert) {
      this.centerDialog();
    }

    this.reformat();
  }

  async onEnableClangFormatExpert(value: boolean) {
    this.enableClangFormatExpert = value;

    localStorage.setItem('enableClangFormatExpert', value.toString());

    if (value) {
      this.centerDialog();
    }
  }

  private centerDialog() {
    this.cdr.detectChanges();
    this.dialog.dialogRef.nativeElement.style.top =
      (window.innerHeight - this.dialog.dialogRef.nativeElement.offsetHeight) /
        2 +
      'px';
    const rect = this.dialog.dialogRef.nativeElement.getBoundingClientRect();
    if (rect.right > window.innerWidth) {
      this.dialog.dialogRef.nativeElement.style.left =
        window.innerWidth -
        this.dialog.dialogRef.nativeElement.offsetWidth +
        'px';
    }
  }

  async onDemangle(mangledName: string) {
    await this.loadWasmDemanglerModule();
    if (this.enableClangFormat) {
      await this.loadWasmFormatterModule();
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

  isLoading(): boolean {
    return (
      this.wasmLoaderDemangler.loading() || this.wasmLoaderFormatter.loading()
    );
  }

  loadYamlFromFile(event: Event) {
    let fileReader = new FileReader();
    fileReader.onload = (e) => {
      this.formatStyle = this.formatter!.deserializeFromYaml(
        fileReader.result!
      );
      this.reformat();
    };
    fileReader.readAsText(
      (event.currentTarget as HTMLInputElement).files!.item(0)!
    );
  }

  loadYamlFromText() {
    this.formatStyle = this.formatter!.deserializeFromYaml(
      this.textClangConfig.nativeElement.value
    );
    this.reformat();
  }

  downloadYaml(event: Event) {
    const newBlob = new Blob(
      [this.formatter!.serializeToYaml(this.formatStyle!)],
      { type: 'application/x-yaml' }
    );
    const data = window.URL.createObjectURL(newBlob);
    const link = document.createElement('a');
    link.href = data;
    link.download = '.clang-format'; // set a name for the file
    link.click();
  }

  saveYamlToText() {
    navigator.clipboard.writeText(
      this.formatter!.serializeToYaml(this.formatStyle!)
    );
  }
}
