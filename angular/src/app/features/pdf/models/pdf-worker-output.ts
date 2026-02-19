export interface WorkerPdfOutput {
    pdfDataURL: ArrayBuffer;
    stdOut: string;
    stdErr: string;
}

export interface WorkerZipOutput {
    zipDataURL: ArrayBuffer;
    stdOut: string;
    stdErr: string;
}

export interface WorkerNumberOutput {
    value: number;
    stdOut: string;
    stdErr: string;
}

export interface WorkerImagePdfOutput {
    pngBytes: ArrayBuffer;
    pageNumber: number;
    stdOut: string;
    stdErr: string;
}

