import {
  ChangeDetectionStrategy,
  Component,
  effect,
  inject,
  signal,
} from '@angular/core';

import { PdfWorkerService } from '../features/pdf/services/pdf-worker.service';

enum ButtonAction {
  None,
  Single,
  Multiple,
}

@Component({
  selector: 'app-main-pdf',
  imports: [],
  templateUrl: './main-pdf.component.html',
  styleUrl: './main-pdf.component.css',
  changeDetection: ChangeDetectionStrategy.OnPush,
})
export class MainPdfComponent {
  private singleFileName: File | null = null;
  private singleFileBuffer: ArrayBuffer | null = null;
  private buttonAction = ButtonAction.None;

  protected readonly status = signal('');
  protected readonly downloadVisilibity = signal('display-none');

  private generatedUrl: string | null = null;
  protected readonly generatedPageUrls = signal([] as string[]);

  protected readonly numberOfPages = signal(0);

  private readonly pdfWorkerService = inject(PdfWorkerService);

  constructor() {
    effect(() => {
      void this.numberOfPages();
      this.generatePages();
    });
  }

  protected onSingleFileSelected(event: Event) {
    const element = event.currentTarget as HTMLInputElement;
    const files = element.files!;
    this.singleFileName = files.item(0);
  }

  private pdfWorker: Worker | null = null;

  protected compress() {
    if (this.singleFileName === null) {
      this.status.set('Select one PDF file.');
      return;
    }

    this.pdfWorker?.terminate();

    const reader = new FileReader();

    reader.onload = async (event) => {
      const arrayBuffer = event.target!.result;
      const blob = new Blob([arrayBuffer!], { type: 'application/pdf' });
      const pdfDataURL = URL.createObjectURL(blob);

      this.status.set('Compress in progress...');
      this.downloadVisilibity.set('display-none');

      try {
        const [pdfWorker, retvalPromise] = this.pdfWorkerService.compressPdf({
          pdfDataURL,
        });
        this.pdfWorker = pdfWorker;
        const retval = await retvalPromise;

        if (retval.stdErr !== "") {
          throw new Error(retval.stdErr)
        }
        if (retval.stdOut !== "") {
          throw new Error(retval.stdOut)
        }

        this.generatedUrl = retval.pdfDataURL;

        this.status.set('Compress done.');
        this.downloadVisilibity.set('display-block');
        this.buttonAction = ButtonAction.Single;
      } catch (error) {
        console.error('Failed while compressing: ', error);
        this.status.set('Failed while compressing.');
      }
    };

    reader.readAsArrayBuffer(this.singleFileName);
  }

  protected split() {
    if (this.singleFileName === null) {
      this.status.set('Select one PDF file.');
      return;
    }

    this.pdfWorker?.terminate();

    const reader = new FileReader();

    reader.onload = async (event) => {
      const arrayBuffer = event.target!.result;
      const blob = new Blob([arrayBuffer!], { type: 'application/pdf' });
      const pdfDataURL = URL.createObjectURL(blob);

      this.status.set('Split in progress...');
      this.downloadVisilibity.set('display-none');

      try {
        const [pdfWorker, retvalPromise] = this.pdfWorkerService.splitPdf({
          pdfDataURL,
        });
        this.pdfWorker = pdfWorker;
        const retval = await retvalPromise;

        if (retval.stdErr !== "") {
          throw new Error(retval.stdErr)
        }
        if (retval.stdOut !== "") {
          throw new Error(retval.stdOut)
        }

        this.generatedUrl = retval.zipDataURL;

        this.status.set('Split done.');
        this.downloadVisilibity.set('display-block');
        this.buttonAction = ButtonAction.Multiple;
      } catch (error) {
        console.error('Failed while spliting:', error);
        this.status.set('Failed while spliting.');
      }
    };

    reader.readAsArrayBuffer(this.singleFileName);
  }

  protected pageCount() {
    if (this.singleFileName === null) {
      this.status.set('Select one PDF file.');
      return;
    }

    this.pdfWorker?.terminate();

    const reader = new FileReader();

    reader.onload = async (event) => {
      const arrayBuffer = event.target!.result;
      const blob = new Blob([arrayBuffer!], { type: 'application/pdf' });
      const pdfDataURL = URL.createObjectURL(blob);

      this.status.set('Counting in progress...');
      this.downloadVisilibity.set('display-none');

      try {
        const [pdfWorker, retvalPromise] = this.pdfWorkerService.pageCountPdf({
          pdfDataURL,
        });
        this.pdfWorker = pdfWorker;
        const retval = await retvalPromise;

        if (retval.stdErr !== "") {
          throw new Error(retval.stdErr)
        }
        if (retval.stdOut !== "" && retval.value !== parseInt(retval.stdOut.trim(), 10)) {
          throw new Error(retval.stdOut)
        }

        this.numberOfPages.set(retval.value);

        this.status.set('Counting done.');
        this.buttonAction = ButtonAction.None;
      } catch (error) {
        console.error('Failed while counting: ', error);
        this.status.set('Failed while counting.');
      }
    };

    reader.readAsArrayBuffer(this.singleFileName);
  }

  private async getPageImage(pageNumber: number): Promise<string> {
    const [worker, retvalPromise] = this.pdfWorkerService.pageImageSized({
      pdfBuffer: this.singleFileBuffer!,
      pageNumber,
      resolution: 10,
    });

    try {
      const retval = await retvalPromise;

      if (retval.stdErr !== "") {
        throw new Error(retval.stdErr)
      }
      if (retval.stdOut !== "") {
        throw new Error(retval.stdOut)
      }

      // Create blob URL in main thread context
      const blob = new Blob([retval.pngBytes], { type: 'image/png' });
      return URL.createObjectURL(blob);
    } finally {
      worker.terminate(); // always terminate, even on error
    }
  }

  public async generatePages(): Promise<void> {
    if (this.singleFileName === null) {
      return;
      throw new Error('No file selected');
    }

    this.singleFileBuffer = await this.singleFileName.arrayBuffer();
    const count = this.numberOfPages();
    const concurrency = Math.min(navigator.hardwareConcurrency, count);
    this.generatedPageUrls().forEach(url => {
      if (url) URL.revokeObjectURL(url);
    });
    this.generatedPageUrls.set(new Array(count).fill(''));

    const indices = Array.from({ length: count }, (_, i) => i);

    const worker = async () => {
      while (indices.length > 0) {
        const i = indices.shift()!;
        let success = false;
        while (!success) {
          try {
            const url = await this.getPageImage(i + 1);
            success = true;
            this.generatedPageUrls.update((urls) => {
              const newUrls = [...urls];
              newUrls[i] = url;
              return newUrls;
            });
          } catch (error) {
            console.error(`Page ${i}: ${error}`);
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
}
