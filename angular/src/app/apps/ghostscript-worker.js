var zip;

import { default as web_gs } from "./gs.js";

function loadScript(module) {
  web_gs(module);
  import("./jszip.min.js").then((JSZip) => {
    zip = new JSZip.default();
  });
}

function compressPdf(dataStruct, responseCallback) {
  // first download the ps data
  var xhr = new XMLHttpRequest();
  xhr.open("GET", dataStruct.psDataURL);
  xhr.responseType = "arraybuffer";
  xhr.onload = function () {
    // release the URL
    self.URL.revokeObjectURL(dataStruct.psDataURL);
    //set up EMScripten environment
    const Module = {
      preRun: [
        function () {
          self.Module.FS.writeFile("input.pdf", new Uint8Array(xhr.response));
        },
      ],
      postRun: [
        function () {
          var uarray = self.Module.FS.readFile("output.pdf", {
            encoding: "binary",
          });
          var blob = new Blob([uarray], { type: "application/octet-stream" });
          var pdfDataURL = self.URL.createObjectURL(blob);
          responseCallback({ pdfDataURL: pdfDataURL, url: dataStruct.url });
        },
      ],
      arguments: [
        "-sDEVICE=pdfwrite",
        "-dCompatibilityLevel=1.4",
        "-dPDFSETTINGS=/ebook",
        "-DNOPAUSE",
        "-dQUIET",
        "-dBATCH",
        "-sOutputFile=output.pdf",
        "input.pdf",
      ],
      print: function (text) {},
      printErr: function (text) {},
      totalDependencies: 0,
      noExitRuntime: 1,
    };
    if (!self.Module) {
      self.Module = Module;
      loadScript(Module);
    } else {
      self.Module["calledRun"] = false;
      self.Module["postRun"] = Module.postRun;
      self.Module["preRun"] = Module.preRun;
      self.Module.callMain();
    }
  };
  xhr.send();
}

function splitPdf(dataStruct, responseCallback) {
  // first download the ps data
  var xhr = new XMLHttpRequest();
  xhr.open("GET", dataStruct.psDataURL);
  xhr.responseType = "arraybuffer";
  xhr.onload = function () {
    // release the URL
    self.URL.revokeObjectURL(dataStruct.psDataURL);
    //set up EMScripten environment
    const Module = {
      preRun: [
        function () {
          self.Module.FS.writeFile("input.pdf", new Uint8Array(xhr.response));
        },
      ],
      postRun: [
        function () {
          var i = 1;

          while (self.Module.FS.analyzePath(i + ".pdf").exists) {
            var fileName = i + ".pdf";
            var uarray = self.Module.FS.readFile(fileName, {
              encoding: "binary",
            });
            var blob = new Blob([uarray], { type: "application/pdf" });

            zip.file(fileName, blob);
            i++;
          }
          zip.generateAsync({ type: "blob" }).then(function (content) {
            var pdfDataURL = self.URL.createObjectURL(content);
            responseCallback({ pdfDataURL: pdfDataURL, url: dataStruct.url });
          });
        },
      ],
      arguments: [
        "-sDEVICE=pdfwrite",
        "-DNOPAUSE",
        "-dQUIET",
        "-dBATCH",
        "-sOutputFile=%d.pdf",
        "input.pdf",
      ],
      print: function (text) {},
      printErr: function (text) {},
      totalDependencies: 0,
      noExitRuntime: 1,
    };
    if (!self.Module) {
      self.Module = Module;
      loadScript(Module);
    } else {
      self.Module["calledRun"] = false;
      self.Module["postRun"] = Module.postRun;
      self.Module["preRun"] = Module.preRun;
      self.Module.callMain();
    }
  };
  xhr.send();
}

self.addEventListener("message", function ({ data: e }) {
  if (e.target === "compress.wasm") {
    compressPdf(e.data, ({ pdfDataURL }) => self.postMessage(pdfDataURL));
    return;
  }

  if (e.target === "split.wasm") {
    splitPdf(e.data, ({ pdfDataURL }) => self.postMessage(pdfDataURL));
    return;
  }
});
