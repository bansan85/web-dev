import {
  Component,
  OnInit,
  ElementRef,
  ViewChild,
  HostListener,
  ChangeDetectorRef,
  ViewEncapsulation,
} from '@angular/core';
import { WasmLoaderFormatterService } from '../wasm-loader-formatter.service';
import { NgIf } from '@angular/common';
import { FormsModule } from '@angular/forms';
import { LucideAngularModule } from 'lucide-angular';
import { DialogPopupComponent } from '../templates/dialog-popup.component';
import { FormatterOptionsComponent } from '../formatter-options/formatter-options.component';
import { TextareaTwoComponent } from '../templates/textarea-two.component';
import { SpinnerLoadingComponent } from '../templates/spinner-loading.component';

import {
  EmbindModule as FormatterModule,
  FormatStyle,
} from '../../assets/web_formatter.js';

@Component({
  selector: 'app-formatter',
  standalone: true,
  imports: [
    DialogPopupComponent,
    FormatterOptionsComponent,
    FormsModule,
    LucideAngularModule,
    NgIf,
    TextareaTwoComponent,
    SpinnerLoadingComponent,
  ],
  templateUrl: './formatter.component.html',
  styleUrl: './formatter.component.css',
  encapsulation: ViewEncapsulation.None,
})
export class AppFormatterComponent implements OnInit {
  formatter?: FormatterModule;

  spinnerSize = 0;

  enableClangFormatExpert = false;

  formatStyle?: FormatStyle;
  emptyStyle?: FormatStyle;

  // Text by pending if text insert while wasm is loading.
  pendingText = false;
  titleLoading = '';

  @ViewChild('newStyle') newStyle!: ElementRef<HTMLSelectElement>;
  @ViewChild('dialog') dialog!: DialogPopupComponent;
  @ViewChild('textClangConfig')
  textClangConfig!: ElementRef<HTMLTextAreaElement>;

  @ViewChild(TextareaTwoComponent) textareaTwo!: TextareaTwoComponent;

  @HostListener('window:resize')
  onResize() {
    this.updateIconSize();
  }

  constructor(
    private wasmLoaderFormatter: WasmLoaderFormatterService,
    private cdr: ChangeDetectorRef
  ) {
    this.format = this.format.bind(this);
  }

  async ngOnInit() {
    this.updateIconSize();

    await this.loadWasmFormatterModule();

    this.enableClangFormatExpert =
      localStorage.getItem('enableClangFormatExpert') === 'true';
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
      this.textareaTwo.inputElement.nativeElement.dispatchEvent(event);
    }
    this.pendingText = false;
  }

  updateIconSize() {
    this.spinnerSize = Math.min(window.innerWidth / 4, window.innerHeight / 2);
  }

  async onEnableClangFormatExpert(event: Event) {
    this.enableClangFormatExpert = (event as any).newState === 'open';

    localStorage.setItem(
      'enableClangFormatExpert',
      this.enableClangFormatExpert.toString()
    );

    this.centerDialog();
  }

  private centerDialog() {
    this.cdr.detectChanges();
    const rect = this.dialog.dialogRef.nativeElement.getBoundingClientRect();
    if (rect.right > window.innerWidth) {
      this.dialog.dialogRef.nativeElement.style.left =
        window.innerWidth -
        this.dialog.dialogRef.nativeElement.offsetWidth +
        'px';
    }
    if (rect.bottom > window.innerHeight) {
      this.dialog.dialogRef.nativeElement.style.top =
        (window.innerHeight -
          this.dialog.dialogRef.nativeElement.offsetHeight) /
          2 +
        'px';
    }
  }

  async format(mangledName: string): Promise<string> {
    await this.loadWasmFormatterModule();
    if (this.formatter) {
      const lines = mangledName.split('\n');
      let demangledName = this.formatter!.formatter(
        mangledName,
        this.formatStyle!
      );
      return demangledName;
    } else {
      this.pendingText = true;
      return '';
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
    this.textareaTwo.inputElement.nativeElement.dispatchEvent(event);

    localStorage.setItem(
      'formatStyle',
      this.formatter!.serializeToYaml(this.formatStyle!)
    );
  }

  isLoading(): boolean {
    if (this.wasmLoaderFormatter.loading()) {
      this.titleLoading = 'formatter';
      return true;
    }
    this.titleLoading = '';
    return false;
  }

  loadYamlFromFile(event: Event) {
    const fileReader = new FileReader();
    fileReader.onload = () => {
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

  downloadYaml() {
    const newBlob = new Blob(
      [this.formatter!.serializeToYaml(this.formatStyle!)],
      { type: 'application/x-yaml' }
    );
    const data = window.URL.createObjectURL(newBlob);
    const link = document.createElement('a');
    link.href = data;
    link.download = '.clang-format';
    link.click();
  }

  saveYamlToText() {
    navigator.clipboard.writeText(
      this.formatter!.serializeToYaml(this.formatStyle!)
    );
  }
}
