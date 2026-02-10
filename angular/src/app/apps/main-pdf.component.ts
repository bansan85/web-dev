import { ChangeDetectionStrategy, Component, signal } from '@angular/core';

import { compressPdf, splitPdf } from './ghostscript-init.js';

enum ButtonAction {
  None,
  Single,
  Multiple,
}

@Component({
  selector: 'app-main-pdf',
  imports: [],
  templateUrl: './main-pdf.component.html',
  styleUrl: './main-pdf.component.css',
  changeDetection: ChangeDetectionStrategy.OnPush,
})
export class MainPdfComponent {
  private singleFileName: File | null = null;
  private buttonAction = ButtonAction.None;

  protected readonly status = signal('');
  protected readonly downloadVisilibity = signal('hide');

  generatedUrl: any = null;

  onSingleFileSelected(event: Event) {
    const element = event.currentTarget as HTMLInputElement;
    const files = element.files!;
    [this.singleFileName] = files;
  }

  compress() {
    if (this.singleFileName === null) {
      this.status.set('Select one PDF file.');
      return;
    }

    const reader = new FileReader();

    reader.onload = async (event) => {
      const arrayBuffer = event.target!.result;
      const blob = new Blob([arrayBuffer!], { type: 'application/pdf' });
      const pdfDataURL = URL.createObjectURL(blob);

      this.status.set('Compress in progress...');
      this.downloadVisilibity.set('hide');

      try {
        this.generatedUrl = await compressPdf({ psDataURL: pdfDataURL });

        this.status.set('Compress done.');
        this.downloadVisilibity.set('show-block');
        this.buttonAction = ButtonAction.Single;
      } catch (error) {
        console.error('Failed while compressing: ', error);
        this.status.set('Failed while compressing.');
      }
    };

    reader.readAsArrayBuffer(this.singleFileName);
  }
  split() {
    if (this.singleFileName === null) {
      this.status.set('Select one PDF file.');
      return;
    }

    const reader = new FileReader();

    reader.onload = async (event) => {
      const arrayBuffer = event.target!.result;
      const blob = new Blob([arrayBuffer!], { type: 'application/pdf' });
      const pdfDataURL = URL.createObjectURL(blob);

      this.status.set('Split in progress...');
      this.downloadVisilibity.set('hide');

      try {
        this.generatedUrl = await splitPdf({ psDataURL: pdfDataURL });

        this.status.set('Split done.');
        this.downloadVisilibity.set('show-block');
        this.buttonAction = ButtonAction.Multiple;
      } catch (error) {
        console.error('Failed while spliting:', error);
        this.status.set('Failed while spliting.');
      }
    };

    reader.readAsArrayBuffer(this.singleFileName);
  }

  download() {
    if (this.generatedUrl) {
      if (this.buttonAction === ButtonAction.Single) {
        const downloadLink = document.createElement('a');
        downloadLink.href = this.generatedUrl;
        downloadLink.download = 'generated.pdf';
        document.body.appendChild(downloadLink);
        downloadLink.click();
        document.body.removeChild(downloadLink);
      } else {
        const downloadLink = document.createElement('a');
        downloadLink.href = this.generatedUrl;
        downloadLink.download = 'generated.zip';
        document.body.appendChild(downloadLink);
        downloadLink.click();
        document.body.removeChild(downloadLink);
      }
    }
  }
}
