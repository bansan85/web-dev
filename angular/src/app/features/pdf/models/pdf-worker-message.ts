import { PdfWorkerInput } from "./pdf-worker-input";

export interface PdfWorkerMessage {
  data: PdfWorkerInput;
  target: string;
}
