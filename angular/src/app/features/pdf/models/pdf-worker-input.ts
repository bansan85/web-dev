export interface PdfWorkerInput {
  pdfDataURL: string;
}

export interface PdfWorkerImageInput {
  pdfDataURL: string;
  pageNumber: number,
  resolution: number,
}

export interface PdfWorkerMessage {
  action: string;
  data: PdfWorkerInput | PdfWorkerImageInput;
}
