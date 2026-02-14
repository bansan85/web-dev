import { Injectable } from '@angular/core';

import { PdfWorkerInput } from '../models/pdf-worker-input';
import { PdfWorkerMessage } from '../models/pdf-worker-message';

@Injectable({
  providedIn: 'root',
})
export class PdfWorkerService {
  private runWorker(
    dataStruct: PdfWorkerInput,
    target: string,
  ): [Worker, Promise<string>] {
    const pdfWorker = new Worker(
      new URL('./ghostscript-worker.js', import.meta.url),
      { type: 'module' },
    );
    pdfWorker.postMessage({ data: dataStruct, target } as PdfWorkerMessage);
    return [
      pdfWorker,
      new Promise((resolve) => {
        const listener = (e: MessageEvent) => {
          resolve(e.data);
          pdfWorker.removeEventListener('message', listener);
        };
        pdfWorker.addEventListener('message', listener);
      }),
    ];
  }

  compressPdf(dataStruct: PdfWorkerInput): [Worker, Promise<string>] {
    return this.runWorker(dataStruct, 'compress.wasm');
  }

  splitPdf(dataStruct: PdfWorkerInput): [Worker, Promise<string>] {
    return this.runWorker(dataStruct, 'split.wasm');
  }
}
