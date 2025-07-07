let worker = null;

export async function compressPdf(
  dataStruct,
  responseCallback,
  progressCallback,
  statusUpdateCallback
) {
  if (worker !== null) {
    worker.terminate();
  }
  worker = new Worker(new URL("./ghostscript-worker.js", import.meta.url), {
    type: "module",
  });
  worker.postMessage({ data: dataStruct, target: "compress.wasm" });
  return new Promise((resolve, reject) => {
    const listener = (e) => {
      resolve(e.data);
      worker.removeEventListener("message", listener);
    };
    worker.addEventListener("message", listener);
  });
}

export async function splitPdf(
  dataStruct,
  responseCallback,
  progressCallback,
  statusUpdateCallback
) {
  if (worker !== null) {
    worker.terminate();
  }
  worker = new Worker(new URL("./ghostscript-worker.js", import.meta.url), {
    type: "module",
  });
  worker.postMessage({ data: dataStruct, target: "split.wasm" });
  return new Promise((resolve, reject) => {
    const listener = (e) => {
      resolve(e.data);
      worker.removeEventListener("message", listener);
    };
    worker.addEventListener("message", listener);
  });
}
