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
      new URL('./ghostscript-worker.ts', import.meta.url),
      { type: 'module' },
    );
    pdfWorker.postMessage({ data: dataStruct, target } as PdfWorkerMessage);

    const promise = new Promise<string>((resolve) => {
      const listener = (e: MessageEvent<string>) => {
        resolve(e.data);
        pdfWorker.removeEventListener('message', listener);
      };
      pdfWorker.addEventListener('message', listener);
    });

    return [pdfWorker, promise];
  }

  compressPdf(dataStruct: PdfWorkerInput): [Worker, Promise<string>] {
    return this.runWorker(dataStruct, 'compress');
  }

  splitPdf(dataStruct: PdfWorkerInput): [Worker, Promise<string>] {
    return this.runWorker(dataStruct, 'split');
  }
}
