import { TestBed } from '@angular/core/testing';

import { WasmLoaderFormatterService } from './wasm-loader-formatter.service';

describe('WasmLoaderFormatterService', () => {
  let service: WasmLoaderFormatterService;

  beforeEach(() => {
    TestBed.configureTestingModule({});
    service = TestBed.inject(WasmLoaderFormatterService);
  });

  it('should be created', () => {
    expect(service).toBeTruthy();
  });
});
