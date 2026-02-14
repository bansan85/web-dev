import JSZip from 'jszip';

import { unknownAssertError } from '../../../apps/shared/interfaces/errors.js';
import { default as webGs } from './gs.js';

declare const self: typeof globalThis & { Module: any };

let zip: JSZip;

interface WorkerData {
  psDataURL: string;
  url: string;
}

interface WorkerMessage {
  target: string;
  data: WorkerData;
}

async function loadScript(module: any): Promise<void> {
  zip = new JSZip();
  await webGs(module);
}

async function compressPdf(
  dataStruct: WorkerData,
  responseCallback: (res: any) => void,
): Promise<void> {
  const response = await fetch(dataStruct.psDataURL);
  const buffer = await response.arrayBuffer();
  self.URL.revokeObjectURL(dataStruct.psDataURL);

  const moduleConfig = {
    preRun: [
      () => {
        self.Module.FS.writeFile('input.pdf', new Uint8Array(buffer));
      },
    ],
    postRun: [
      () => {
        const uarray = self.Module.FS.readFile('output.pdf', {
          encoding: 'binary',
        });
        const blob = new Blob([uarray], { type: 'application/octet-stream' });
        const pdfDataURL = self.URL.createObjectURL(blob);
        responseCallback({ pdfDataURL, url: dataStruct.url });
      },
    ],
    arguments: [
      '-sDEVICE=pdfwrite',
      '-dCompatibilityLevel=1.4',
      '-dPDFSETTINGS=/ebook',
      '-DNOPAUSE',
      '-dQUIET',
      '-dBATCH',
      '-sOutputFile=output.pdf',
      'input.pdf',
    ],
    print: () => {
      // Can't disable print method when calling ghostscript.
    },
    printErr: () => {
      // Can't disable print method when calling ghostscript.
    },
    totalDependencies: 0,
    noExitRuntime: 1,
  };

  if (self.Module) {
    self.Module.calledRun = false;
    self.Module.postRun = moduleConfig.postRun;
    self.Module.preRun = moduleConfig.preRun;
    self.Module.callMain();
  } else {
    self.Module = moduleConfig;
    await loadScript(moduleConfig);
  }
}

async function splitPdf(
  dataStruct: WorkerData,
  responseCallback: (res: any) => void,
): Promise<void> {
  const response = await fetch(dataStruct.psDataURL);
  const buffer = await response.arrayBuffer();
  self.URL.revokeObjectURL(dataStruct.psDataURL);

  const moduleConfig = {
    preRun: [
      () => {
        self.Module.FS.writeFile('input.pdf', new Uint8Array(buffer));
      },
    ],
    postRun: [
      async () => {
        let i = 1;
        while (self.Module.FS.analyzePath(`${i}.pdf`).exists) {
          const fileName = `${i}.pdf`;
          const uarray = self.Module.FS.readFile(fileName, {
            encoding: 'binary',
          });
          zip.file(fileName, new Blob([uarray], { type: 'application/pdf' }));
          i += 1;
        }
        const content = await zip.generateAsync({ type: 'blob' });
        const pdfDataURL = self.URL.createObjectURL(content);
        responseCallback({ pdfDataURL, url: dataStruct.url });
      },
    ],
    arguments: [
      '-sDEVICE=pdfwrite',
      '-DNOPAUSE',
      '-dQUIET',
      '-dBATCH',
      '-sOutputFile=%d.pdf',
      'input.pdf',
    ],
    print: () => {
      // Can't disable print method when calling ghostscript.
    },
    printErr: () => {
      // Can't disable print method when calling ghostscript.
    },
    totalDependencies: 0,
    noExitRuntime: 1,
  };

  if (self.Module) {
    self.Module.calledRun = false;
    self.Module.postRun = moduleConfig.postRun;
    self.Module.preRun = moduleConfig.preRun;
    self.Module.callMain();
  } else {
    self.Module = moduleConfig;
    await loadScript(moduleConfig);
  }
}

self.addEventListener('message', (e: MessageEvent<WorkerMessage>) => {
  const { target, data } = e.data;

  if (target === 'compress') {
    compressPdf(data, ({ pdfDataURL }) => {
      self.postMessage(pdfDataURL);
    })
      .then(() => {
        //
      })
      .catch((err: unknown) => {
        throw unknownAssertError(err);
      });
  } else if (target === 'split') {
    splitPdf(data, ({ pdfDataURL }) => {
      self.postMessage(pdfDataURL);
    })
      .then(() => {
        //
      })
      .catch((err: unknown) => {
        throw unknownAssertError(err);
      });
  }
});
