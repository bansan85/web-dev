import {
  ChangeDetectionStrategy,
  ChangeDetectorRef,
  Component,
  computed,
  ElementRef,
  HostListener,
  inject,
  OnInit,
  signal,
  viewChild,
  ViewEncapsulation,
} from '@angular/core';
import { FormsModule } from '@angular/forms';
import { LucideAngularModule } from 'lucide-angular';

import {
  EmbindModule as FormatterModule,
  FormatStyle,
} from '../../assets/web_formatter.js';
import { FormatterOptionsComponent } from '../formatter-options/formatter-options.component';
import { GithubMarkInlineComponent } from '../img/github-mark-inline.component.js';
import { DialogPopupComponent } from '../templates/dialog-popup.component';
import { SpinnerLoadingComponent } from '../templates/spinner-loading.component';
import { TextareaTwoComponent } from '../templates/textarea-two.component';
import { WasmLoaderFormatterService } from '../wasm-loader-formatter.service';
import { assertError } from './shared/interfaces/errors.js';

@Component({
  selector: 'app-formatter',
  imports: [
    DialogPopupComponent,
    FormatterOptionsComponent,
    FormsModule,
    LucideAngularModule,
    TextareaTwoComponent,
    SpinnerLoadingComponent,
    GithubMarkInlineComponent,
  ],
  templateUrl: './formatter.component.html',
  styleUrl: './formatter.component.css',
  changeDetection: ChangeDetectionStrategy.OnPush,
  encapsulation: ViewEncapsulation.None,
})
export class AppFormatterComponent implements OnInit {
  protected formatter?: FormatterModule;

  protected readonly spinnerSize = signal(0);

  protected readonly enableClangFormatExpert = signal(false);

  protected formatStyle?: FormatStyle;
  protected emptyStyle?: FormatStyle;

  // Text by pending if text insert while wasm is loading.
  private pendingText = false;
  protected titleLoading = '';

  private readonly newStyle = viewChild.required<ElementRef<HTMLSelectElement>>('newStyle');
  private readonly dialog = viewChild.required<DialogPopupComponent>('dialog');
  private readonly textClangConfig = viewChild.required<ElementRef<HTMLTextAreaElement>>('textClangConfig');

  private readonly textareaTwo = viewChild.required(TextareaTwoComponent);

  @HostListener('window:resize')
  onResize() {
    this.updateIconSize();
  }

  private readonly wasmLoaderFormatter = inject(WasmLoaderFormatterService);
  private readonly cdr = inject(ChangeDetectorRef);

  constructor(
  ) {
    this.format = this.format.bind(this);
  }

  async ngOnInit() {
    this.updateIconSize();

    await this.loadWasmFormatterModule();

    this.enableClangFormatExpert.set(
      localStorage.getItem('enableClangFormatExpert') === 'true');
  }

  private async loadWasmFormatterModule() {
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
      this.textareaTwo().inputElement().nativeElement.dispatchEvent(event);
    }
    this.pendingText = false;
  }

  private updateIconSize() {
    this.spinnerSize.set(Math.min(window.innerWidth / 4, window.innerHeight / 2));
  }

  protected onEnableClangFormatExpert(event: Event) {
    this.enableClangFormatExpert.set((event as any).newState === 'open');

    localStorage.setItem(
      'enableClangFormatExpert',
      this.enableClangFormatExpert().toString()
    );

    this.centerDialog();
  }

  private centerDialog() {
    this.cdr.detectChanges();
    const rect = this.dialog().dialogRef().nativeElement.getBoundingClientRect();
    if (rect.right > window.innerWidth) {
      this.dialog().dialogRef().nativeElement.style.left =
        `${window.innerWidth -
        this.dialog().dialogRef().nativeElement.offsetWidth
        }px`;
    }
    if (rect.bottom > window.innerHeight) {
      this.dialog().dialogRef().nativeElement.style.top =
        `${(window.innerHeight -
          this.dialog().dialogRef().nativeElement.offsetHeight) /
        2
        }px`;
    }
  }

  protected async format(mangledName: string): Promise<string> {
    await this.loadWasmFormatterModule();
    if (this.formatter) {
      const demangledName = this.formatter.formatter(
        mangledName,
        this.formatStyle!
      );
      return demangledName;
    } else {
      this.pendingText = true;
      return '';
    }
  }

  protected loadStyle() {
    switch (this.newStyle().nativeElement.value) {
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
      default:
        throw assertError(`Unknown style ${this.newStyle().nativeElement.value}.`);
    }

    this.reformat();
  }

  protected reformat() {
    const event = new Event('input', { bubbles: true });
    this.textareaTwo().inputElement().nativeElement.dispatchEvent(event);

    localStorage.setItem(
      'formatStyle',
      this.formatter!.serializeToYaml(this.formatStyle!)
    );
  }

  protected readonly isLoading = computed(() => {
    if (this.wasmLoaderFormatter.isLoading()) {
      this.titleLoading = 'formatter';
      return true;
    }
    this.titleLoading = '';
    return false;
  });

  protected loadYamlFromFile(event: Event) {
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

  protected loadYamlFromText() {
    this.formatStyle = this.formatter!.deserializeFromYaml(
      this.textClangConfig().nativeElement.value
    );
    this.reformat();
  }

  protected downloadYaml() {
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

  protected async saveYamlToText() {
    await navigator.clipboard.writeText(
      this.formatter!.serializeToYaml(this.formatStyle!)
    );
  }
}
