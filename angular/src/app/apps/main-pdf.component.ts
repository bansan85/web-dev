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
  private buttonAction = ButtonAction.None;

  protected readonly status = signal('');
  protected readonly downloadVisilibity = signal('display-none');

  private generatedUrl: string | null = null;
  protected readonly generatedPageUrls = signal([] as string[]);

  protected readonly numberOfPages = signal(0);

  private readonly pdfWorkerService = inject(PdfWorkerService);

  constructor() {
    effect(() => {
      void this.generatePages();
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
        console.log(retval.value);
        this.numberOfPages.set(retval.value);

        this.status.set('Counting done.');
        this.buttonAction = ButtonAction.None;
      } catch (error) {
        console.error('Failed while Counting: ', error);
        this.status.set('Failed while Counting.');
      }
    };

    reader.readAsArrayBuffer(this.singleFileName);
  }

  protected async getPageImage(pageNumber: number): Promise<string> {
    console.log(pageNumber);
    if (!this.singleFileName) {
      throw new Error('No file selected');
    }

    return new Promise((resolve, reject) => {
      const reader = new FileReader();

      reader.onload = async (event) => {
        try {
          const arrayBuffer = event.target?.result as ArrayBuffer;
          const blob = new Blob([arrayBuffer], { type: 'image/png' });
          const pdfDataURL = URL.createObjectURL(blob);

          const [_, retvalPromise] = this.pdfWorkerService.pageImageSized({
            pdfDataURL,
            pageNumber,
            resolution: 10,
          });

          const retval = await retvalPromise;
          resolve(retval.pngDataURL);
        } catch (error) {
          reject(error instanceof Error ? error : new Error(String(error)));
        }
      };

      reader.onerror = () => {
        reject(new Error('FileReader error'));
      };
      reader.readAsArrayBuffer(this.singleFileName!);
    });
  }

  public async generatePages(): Promise<void> {
    const count = this.numberOfPages();
    const concurrency = 8;
    this.generatedPageUrls.set(new Array(count).fill(''));

    const indices = Array.from({ length: count }, (_, i) => i);

    const worker = async () => {
      while (indices.length > 0) {
        const i = indices.shift()!;
        try {
          const url = await this.getPageImage(i + 1);
          this.generatedPageUrls.update((urls) => {
            const newUrls = [...urls];
            newUrls[i] = url;
            return newUrls;
          });
        } catch (error) {
          console.error(error);
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
