import { ChangeDetectionStrategy, Component, signal } from '@angular/core';

enum ButtonAction {
  None,
  Single,
  Multiple,
}

interface PdfWorkerInput {
  psDataURL: string;
}

interface PdfWorkerMessage {
  data: PdfWorkerInput;
  target: string;
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

  private generatedUrl: any = null;

  protected onSingleFileSelected(event: Event) {
    const element = event.currentTarget as HTMLInputElement;
    const files = element.files!;
    [this.singleFileName] = files;
  }

  private pdfWorker: Worker | null = null;

  private runWorker(dataStruct: PdfWorkerInput, target: string): Promise<unknown> {
    this.pdfWorker?.terminate();
    this.pdfWorker = new Worker(
      new URL('./ghostscript-worker.js', import.meta.url),
      { type: 'module' },
    );
    this.pdfWorker.postMessage({ data: dataStruct, target } as PdfWorkerMessage);
    return new Promise((resolve) => {
      const listener = (e: MessageEvent) => {
        resolve(e.data);
        this.pdfWorker!.removeEventListener('message', listener);
      };
      this.pdfWorker!.addEventListener('message', listener);
    });
  }

  private compressPdf(dataStruct: PdfWorkerInput): Promise<unknown> {
    return this.runWorker(dataStruct, 'compress.wasm');
  }

  private splitPdf(dataStruct: PdfWorkerInput): Promise<unknown> {
    return this.runWorker(dataStruct, 'split.wasm');
  }

  protected compress() {
    if (this.singleFileName === null) {
      this.status.set('Select one PDF file.');
      return;
    }

    const reader = new FileReader();

    reader.onload = async (event) => {
      const arrayBuffer = event.target!.result;
      const blob = new Blob([arrayBuffer!], { type: 'application/pdf' });
      const pdfDataURL = URL.createObjectURL(blob);

      this.status.set('Compress in progress...');
      this.downloadVisilibity.set('display-none');

      try {
        this.generatedUrl = await this.compressPdf({ psDataURL: pdfDataURL });

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

    const reader = new FileReader();

    reader.onload = async (event) => {
      const arrayBuffer = event.target!.result;
      const blob = new Blob([arrayBuffer!], { type: 'application/pdf' });
      const pdfDataURL = URL.createObjectURL(blob);

      this.status.set('Split in progress...');
      this.downloadVisilibity.set('display-none');

      try {
        this.generatedUrl = await this.splitPdf({ psDataURL: pdfDataURL });

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
