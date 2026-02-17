export interface PdfWorkerInput {
  pdfDataURL: string;
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
