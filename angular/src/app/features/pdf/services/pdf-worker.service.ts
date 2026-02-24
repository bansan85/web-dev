import { Injectable } from '@angular/core';

import {
  PdfWorkerImageInput,
  PdfWorkerInput,
  PdfWorkerMessage,
} from '../models/pdf-worker-input';
import {
  WorkerImagePdfOutput,
  WorkerNumberOutput,
  WorkerPdfOutput,
  WorkerZipOutput,
} from '../models/pdf-worker-output';

@Injectable({
  providedIn: 'root',
})
export class PdfWorkerService {
  private runWorker<T>(dataStruct: unknown, action: string): Promise<T> {
    const pdfWorker = new Worker(
      new URL('./ghostscript-worker.ts', import.meta.url),
      { type: 'module' },
    );
    pdfWorker.postMessage({ action, data: dataStruct } as PdfWorkerMessage);

    const promise = new Promise<T>((resolve) => {
      const listener = (e: MessageEvent<T>) => {
        resolve(e.data);
        pdfWorker.removeEventListener('message', listener);
        pdfWorker.terminate();
      };
      pdfWorker.addEventListener('message', listener);
    });

    return promise;
  }

  compressPdf(dataStruct: PdfWorkerInput): Promise<WorkerPdfOutput> {
    return this.runWorker(dataStruct, 'compress');
  }

  splitPdf(dataStruct: PdfWorkerInput): Promise<WorkerZipOutput> {
    return this.runWorker(dataStruct, 'split');
  }

  pageCountPdf(dataStruct: PdfWorkerInput): Promise<WorkerNumberOutput> {
    return this.runWorker(dataStruct, 'pageCount');
  }

  pageImageSized(
    dataStruct: PdfWorkerImageInput,
  ): Promise<WorkerImagePdfOutput> {
    return this.runWorker(dataStruct, 'pageImageSized');
  }
}
