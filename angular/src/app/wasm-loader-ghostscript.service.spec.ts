import { TestBed } from '@angular/core/testing';

import {
  WasmLoaderGhostscriptService,
} from './wasm-loader-ghostscript.service';

describe('WasmLoaderGhostscriptService', () => {
  let service: WasmLoaderGhostscriptService;

  beforeEach(() => {
    TestBed.configureTestingModule({});
    service = TestBed.inject(WasmLoaderGhostscriptService);
  });

  it('should be created', () => {
    expect(service).toBeTruthy();
  });

  it('check ghostscript', async () => {
    await service.preload();
  });
});
