let worker = null;

export async function splitPdf( dataStruct,
                                   responseCallback,
                                   progressCallback,
                                   statusUpdateCallback){
  if (worker !== null) {
    worker.terminate();
  }
  worker = new Worker(
    new URL('./split-worker.js', import.meta.url),
    {type: 'module'}
  );
  worker.postMessage({ data: dataStruct, target: 'wasm'});
  return new Promise((resolve, reject)=>{
    const listener = (e) => {
      resolve(e.data)
      worker.removeEventListener('message', listener)
    }
    worker.addEventListener('message', listener);
  })

}



