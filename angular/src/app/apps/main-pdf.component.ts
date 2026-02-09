import { ChangeDetectionStrategy, Component, ElementRef,viewChild } from '@angular/core';

import { compressPdf, splitPdf } from './ghostscript-init.js';

@Component({
  selector: 'app-main-pdf',
  imports: [],
  templateUrl: './main-pdf.component.html',
  styleUrl: './main-pdf.component.css',
  changeDetection: ChangeDetectionStrategy.OnPush,
})
export class MainPdfComponent {
  private readonly fileInput = viewChild.required<ElementRef<HTMLInputElement>>('fileInput');
  private readonly status = viewChild.required<ElementRef<HTMLParagraphElement>>('status');
  private readonly singleDlBtn = viewChild.required<ElementRef<HTMLButtonElement>>('singleDlBtn');
  private readonly multipleDlBtn = viewChild.required<ElementRef<HTMLButtonElement>>('multipleDlBtn');

  generatedUrl: any = null;

  compress() {
    if (this.fileInput().nativeElement.files!.length === 0) {
      this.status().nativeElement.textContent =
        'Veuillez sélectionner un fichier PDF.';
      return;
    }

    const [file] = this.fileInput().nativeElement.files!;
    const reader = new FileReader();

    reader.onload = async (event) => {
      const arrayBuffer = event.target!.result;
      const blob = new Blob([arrayBuffer!], { type: 'application/pdf' });
      const pdfDataURL = URL.createObjectURL(blob);

      this.status().nativeElement.textContent = 'Compress in progress...';
      this.singleDlBtn().nativeElement.style.display = 'none';
      this.multipleDlBtn().nativeElement.style.display = 'none';

      try {
        this.generatedUrl = await compressPdf({ psDataURL: pdfDataURL });

        this.status().nativeElement.textContent = 'Compress done.';
        this.singleDlBtn().nativeElement.style.display = 'block';
      } catch (error) {
        console.error('Failed while compressing: ', error);
        this.status().nativeElement.textContent = 'Failed while compressing.';
      }
    };

    reader.readAsArrayBuffer(file);
  }
  split() {
    if (this.fileInput().nativeElement.files!.length === 0) {
      this.status().nativeElement.textContent = 'Veuillez sélectionner un fichier PDF.';
      return;
    }

    const [file] = this.fileInput().nativeElement.files!;
    const reader = new FileReader();

    reader.onload = async (event)=> {
      const arrayBuffer = event.target!.result;
      const blob = new Blob([arrayBuffer!], { type: 'application/pdf' });
      const pdfDataURL = URL.createObjectURL(blob);

      this.status().nativeElement.textContent = 'Split in progress...';
      this.singleDlBtn().nativeElement.style.display = 'none';
      this.multipleDlBtn().nativeElement.style.display = 'none';

      try {
        this.generatedUrl = await splitPdf({ psDataURL: pdfDataURL });

        this.status().nativeElement.textContent = 'Split done.';
        this.multipleDlBtn().nativeElement.style.display = 'block';
      } catch (error) {
        console.error('Failed while spliting:', error);
        this.status().nativeElement.textContent = 'Failed while spliting.';
      }
    };

    reader.readAsArrayBuffer(file);
  }
  singleDownload() {
    if (this.generatedUrl) {
      const downloadLink = document.createElement("a");
      downloadLink.href = this.generatedUrl;
      downloadLink.download = "generated.pdf";
      document.body.appendChild(downloadLink);
      downloadLink.click();
      document.body.removeChild(downloadLink);
  }
  }
  multipleDownload() {
    if (this.generatedUrl) {
      const downloadLink = document.createElement("a");
      downloadLink.href = this.generatedUrl;
      downloadLink.download = "generated.zip";
      document.body.appendChild(downloadLink);
      downloadLink.click();
      document.body.removeChild(downloadLink);
  }
}
}
