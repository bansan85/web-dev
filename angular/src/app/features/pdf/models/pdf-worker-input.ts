export interface PdfWorkerInput {
  pdfBuffer: ArrayBuffer;
}

export interface PdfWorkerImageInput {
  pdfBuffer: ArrayBuffer;
  pageNumber: number,
  resolution: number,
}

export interface PdfWorkerMessage {
  action: string;
  data: PdfWorkerInput | PdfWorkerImageInput;
}
