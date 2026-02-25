import {
  ChangeDetectionStrategy,
  Component,
  computed,
  HostListener,
  inject,
  model,
  OnInit,
  signal,
} from '@angular/core';
import { FormsModule } from '@angular/forms';

import { PdfWorkerService } from '../features/pdf/services/pdf-worker.service';
import { TabItem } from '../features/tab/components/tab-item';
import { Tabs } from '../features/tab/components/tabs';
import { GithubMarkInlineComponent } from '../img/github-mark-inline.component';
import { SpinnerLoadingComponent } from '../templates/spinner-loading.component';
import { WasmLoaderGhostscriptService } from '../wasm-loader-ghostscript.service';

enum ButtonAction {
  None,
  Single,
  Multiple,
}

@Component({
  selector: 'app-main-pdf',
  imports: [
    GithubMarkInlineComponent,
    TabItem,
    Tabs,
    SpinnerLoadingComponent,
    FormsModule,
  ],
  templateUrl: './main-pdf.component.html',
  styleUrl: './main-pdf.component.css',
  changeDetection: ChangeDetectionStrategy.OnPush,
})
export class MainPdfComponent implements OnInit {
  private singleFileName: File | null = null;
  private singleFileBuffer: ArrayBuffer | null = null;
  private buttonAction = ButtonAction.None;

  protected readonly status = signal('');
  protected readonly downloadVisilibity = signal('display-none');
  protected readonly enabledSplit = model(false);

  private generatedUrl: string | null = null;
  protected readonly generatedPageUrls = signal<string[]>([]);

  protected readonly numberOfPages = signal(0);

  private readonly pdfWorkerService = inject(PdfWorkerService);

  private readonly wasmLoaderGhostscript = inject(WasmLoaderGhostscriptService);
  protected titleLoading = '';
  protected readonly spinnerSize = signal(0);

  protected async onSingleFileSelected(event: Event) {
    const element = event.currentTarget as HTMLInputElement;
    const files = element.files!;
    this.singleFileName = files.item(0)!;
    this.singleFileBuffer = await this.singleFileName.arrayBuffer();
  }

  protected async compress() {
    if (this.singleFileName === null) {
      this.status.set('Select one PDF file.');
      return;
    }

    this.status.set('Compress in progress...');
    this.downloadVisilibity.set('display-none');

    try {
      const retval = await this.pdfWorkerService.compressPdf({
        pdfBuffer: this.singleFileBuffer!,
      });

      if (retval.stdErr !== '') {
        throw new Error(retval.stdErr);
      }
      if (retval.stdOut !== '') {
        throw new Error(retval.stdOut);
      }

      const blob = new Blob([retval.pdfDataURL], { type: 'application/pdf' });
      if (this.generatedUrl !== null) {
        URL.revokeObjectURL(this.generatedUrl);
      }
      this.generatedUrl = URL.createObjectURL(blob);

      this.status.set('Compress done.');
      this.downloadVisilibity.set('display-block');
      this.buttonAction = ButtonAction.Single;
    } catch (error) {
      console.error('Failed while compressing: ', error);
      this.status.set('Failed while compressing.');
    }
  }

  protected async split() {
    if (this.singleFileName === null) {
      this.status.set('Select one PDF file.');
      return;
    }

    this.status.set('Split in progress...');
    this.downloadVisilibity.set('display-none');

    try {
      const retval = await this.pdfWorkerService.splitPdf({
        pdfBuffer: this.singleFileBuffer!,
      });

      if (retval.stdErr !== '') {
        throw new Error(retval.stdErr);
      }
      if (retval.stdOut !== '') {
        throw new Error(retval.stdOut);
      }

      const blob = new Blob([retval.zipDataURL], { type: 'application/zip' });
      if (this.generatedUrl !== null) {
        URL.revokeObjectURL(this.generatedUrl);
      }
      this.generatedUrl = URL.createObjectURL(blob);

      this.status.set('Split done.');
      this.downloadVisilibity.set('display-block');
      this.buttonAction = ButtonAction.Multiple;
    } catch (error) {
      console.error('Failed while spliting:', error);
      this.status.set('Failed while spliting.');
    }
  }

  protected async pageCount() {
    if (this.singleFileName === null) {
      this.status.set('Select one PDF file.');
      return;
    }

    this.status.set('Counting in progress...');
    this.downloadVisilibity.set('display-none');

    try {
      const retval = await this.pdfWorkerService.pageCountPdf({
        pdfBuffer: this.singleFileBuffer!,
      });

      if (retval.stdErr !== '') {
        throw new Error(retval.stdErr);
      }
      if (
        retval.stdOut !== '' &&
        retval.value !== parseInt(retval.stdOut.trim(), 10)
      ) {
        throw new Error(retval.stdOut);
      }

      this.numberOfPages.set(retval.value);

      this.status.set('Counting done.');
      this.buttonAction = ButtonAction.None;
    } catch (error) {
      console.error('Failed while counting: ', error);
      this.status.set('Failed while counting.');
    }

    await this.generatePages();
  }

  private async getPageImage(pageNumber: number): Promise<string> {
    const retval = await this.pdfWorkerService.pageImageSized({
      pdfBuffer: this.singleFileBuffer!,
      pageNumber,
      resolution: 10,
    });

    if (retval.stdErr !== '') {
      throw new Error(retval.stdErr);
    }
    if (retval.stdOut !== '') {
      throw new Error(retval.stdOut);
    }

    // Create blob URL in main thread context to allow worker termination.
    const blob = new Blob([retval.pngBytes], { type: 'image/png' });
    return URL.createObjectURL(blob);
  }

  public async generatePages(): Promise<void> {
    if (this.singleFileName === null) {
      throw new Error('No file selected');
    }

    const count = this.numberOfPages();
    const concurrency = Math.min(navigator.hardwareConcurrency, count);
    this.generatedPageUrls().forEach((url) => {
      if (url) URL.revokeObjectURL(url);
    });
    this.generatedPageUrls.set(new Array(count).fill(''));

    const indices = Array.from({ length: count }, (_, i) => i);

    const worker = async () => {
      while (indices.length > 0) {
        const i = indices.shift()!;
        for (let retry = 0; retry < 10; retry += 1) {
          try {
            // eslint-disable-next-line no-await-in-loop
            const url = await this.getPageImage(i + 1);
            this.generatedPageUrls.update((urls) => {
              const newUrls = [...urls];
              newUrls[i] = url;
              return newUrls;
            });
            break;
          } catch (error) {
            console.error(`Page ${i + 1}: ${error}`);
          }
        }
      }
    };

    const pool = Array.from({ length: concurrency }, () => worker());
    await Promise.all(pool);
  }

  protected download() {
    if (this.generatedUrl) {
      if (this.buttonAction === ButtonAction.Single) {
        const downloadLink = document.createElement('a');
        downloadLink.href = this.generatedUrl;
        downloadLink.download = 'generated.pdf';
        document.body.appendChild(downloadLink);
        downloadLink.click();
        document.body.removeChild(downloadLink);
      } else {
        const downloadLink = document.createElement('a');
        downloadLink.href = this.generatedUrl;
        downloadLink.download = 'generated.zip';
        document.body.appendChild(downloadLink);
        downloadLink.click();
        document.body.removeChild(downloadLink);
      }
    }
  }

  protected readonly isLoading = computed(() => {
    if (this.wasmLoaderGhostscript.isLoading()) {
      this.titleLoading = 'ghostscript';
      return true;
    }
    this.titleLoading = '';
    return false;
  });

  private updateIconSize() {
    this.spinnerSize.set(
      Math.min(window.innerWidth / 4, window.innerHeight / 2),
    );
  }

  async ngOnInit() {
    this.updateIconSize();

    await this.wasmLoaderGhostscript.preload();
  }

  @HostListener('window:resize')
  onResize() {
    this.updateIconSize();
  }
}
