export interface WorkerPdfOutput {
    pdfDataURL: string;
    stdOut: string;
    stdErr: string;
}

export interface WorkerZipOutput {
    zipDataURL: string;
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

