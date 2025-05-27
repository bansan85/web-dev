function loadScript() {
  import("./gs.js");
}

var Module;

function compressPdf(dataStruct, responseCallback) {
  // first download the ps data
  var xhr = new XMLHttpRequest();
  xhr.open("GET", dataStruct.psDataURL);
  xhr.responseType = "arraybuffer";
  xhr.onload = function () {
    // release the URL
    self.URL.revokeObjectURL(dataStruct.psDataURL);
    //set up EMScripten environment
    Module = {
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
      loadScript();
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
  // e.data contains the message sent to the worker.
  if (e.target !== "wasm") {
    return;
  }
  compressPdf(e.data, ({ pdfDataURL }) => self.postMessage(pdfDataURL));
});
