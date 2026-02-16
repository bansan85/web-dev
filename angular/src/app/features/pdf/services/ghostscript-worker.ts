import JSZip from 'jszip';

import { unknownAssertError } from '../../../apps/shared/interfaces/errors.js';
import { default as webGs } from './gs.js';
import { WorkerImagePdfOutput, WorkerNumberOutput, WorkerPdfOutput, WorkerZipOutput } from '../models/pdf-worker-output.js';

declare const self: typeof globalThis & { Module: any };

let zip: JSZip;

interface WorkerData {
  pdfDataURL: string;
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
  responseCallback: (res: WorkerPdfOutput) => void,
): Promise<void> {
  const response = await fetch(dataStruct.pdfDataURL);
  const buffer = await response.arrayBuffer();
  self.URL.revokeObjectURL(dataStruct.pdfDataURL);

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
        responseCallback({ pdfDataURL });
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
  responseCallback: (res: WorkerZipOutput) => void,
): Promise<void> {
  const response = await fetch(dataStruct.pdfDataURL);
  const buffer = await response.arrayBuffer();
  self.URL.revokeObjectURL(dataStruct.pdfDataURL);

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
        const zipDataURL = self.URL.createObjectURL(content);
        responseCallback({ zipDataURL });
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

async function getPageCount(
  dataStruct: WorkerData,
  responseCallback: (res: WorkerNumberOutput) => void,
): Promise<void> {
  const response = await fetch(dataStruct.pdfDataURL);
  const buffer = await response.arrayBuffer();
  self.URL.revokeObjectURL(dataStruct.pdfDataURL);

  let output = '';

  const moduleConfig = {
    preRun: [
      () => {
        self.Module.FS.writeFile('input.pdf', new Uint8Array(buffer));
      },
    ],
    postRun: [
      () => {
        const value = parseInt(output.trim(), 10);
        responseCallback({ value });
      },
    ],
    arguments: [
      '-q',
      '-dNODISPLAY',
      '-dNOSAFER',
      '-c',
      '(input.pdf) (r) file runpdfbegin pdfpagecount = quit',
    ],
    print: (text: string) => {
      output += text;
    },
    printErr: () => {
      //
    },
    totalDependencies: 0,
    noExitRuntime: 1,
  };

  if (self.Module) {
    self.Module.calledRun = false;
    self.Module.postRun = moduleConfig.postRun;
    self.Module.preRun = moduleConfig.preRun;
    self.Module.print = moduleConfig.print;
    self.Module.callMain(moduleConfig.arguments);
  } else {
    self.Module = moduleConfig;
    await loadScript(moduleConfig);
  }
}

async function getPageImageSized(
  dataStruct: WorkerData,
  pageNumber: number,
  resolution: number,
  responseCallback: (res: WorkerImagePdfOutput) => void,
): Promise<void> {
  const response = await fetch(dataStruct.pdfDataURL);
  const buffer = await response.arrayBuffer();
  self.URL.revokeObjectURL(dataStruct.pdfDataURL);

  const moduleConfig = {
    preRun: [
      () => {
        self.Module.FS.writeFile('input.pdf', new Uint8Array(buffer));
      },
    ],
    postRun: [
      () => {
        const fileName = 'output.png';
        if (self.Module.FS.analyzePath(fileName).exists) {
          const uarray = self.Module.FS.readFile(fileName);
          const blob = new Blob([uarray], { type: 'image/png' });
          const pngDataURL = self.URL.createObjectURL(blob);
          responseCallback({ pngDataURL, pageNumber });
        }
      },
    ],
    arguments: [
      '-sDEVICE=fpng',
      `-r${resolution}`,
      `-dFirstPage=${pageNumber}`,
      `-dLastPage=${pageNumber}`,
      '-dNOPAUSE',
      '-dBATCH',
      '-dQUIET',
      '-dNOSAFER',
      '-sOutputFile=output.png',
      'input.pdf',
    ],
    print: (text: string) => {
      console.log(text);
      //
    },
    printErr: (text: string) => {
      console.error(text);
      //
    },
    totalDependencies: 0,
    noExitRuntime: 1,
  };

  if (self.Module) {
    self.Module.calledRun = false;
    self.Module.postRun = moduleConfig.postRun;
    self.Module.preRun = moduleConfig.preRun;
    self.Module.callMain(moduleConfig.arguments);
  } else {
    self.Module = moduleConfig;
    await loadScript(moduleConfig);
  }
}

self.addEventListener('message', (e: MessageEvent<WorkerMessage>) => {
  const { target, data } = e.data;

  if (target === 'compress') {
    compressPdf(data, (retval) => {
      self.postMessage(retval);
    })
      .then(() => {
        //
      })
      .catch((err: unknown) => {
        throw unknownAssertError(err);
      });
  } else if (target === 'split') {
    splitPdf(data, (retval) => {
      self.postMessage(retval);
    })
      .then(() => {
        //
      })
      .catch((err: unknown) => {
        throw unknownAssertError(err);
      });
  } else if (target === 'pageCount') {
    getPageCount(data, (retval) => {
      self.postMessage(retval);
    })
      .then(() => {
        //
      })
      .catch((err: unknown) => {
        throw unknownAssertError(err);
      });
  } else if (target === 'pageImageSized') {
    getPageImageSized(data, 2, 50, (retval) => {
      self.postMessage(retval);
    })
      .then(() => {
        //
      })
      .catch((err: unknown) => {
        throw unknownAssertError(err);
      });
  }
});
