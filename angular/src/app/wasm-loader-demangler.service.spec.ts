import { TestBed } from '@angular/core/testing';

import { WasmLoaderDemanglerService } from './wasm-loader-demangler.service';

describe('WasmLoaderDemanglerService', () => {
  let service: WasmLoaderDemanglerService;

  beforeEach(() => {
    TestBed.configureTestingModule({});
    service = TestBed.inject(WasmLoaderDemanglerService);
  });

  it('should be created', () => {
    expect(service).toBeTruthy();
  });
});
