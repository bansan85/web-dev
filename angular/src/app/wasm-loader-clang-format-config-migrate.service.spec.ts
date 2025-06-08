import { TestBed } from '@angular/core/testing';

import {
  WasmLoaderClangFormatConfigMigrateService,
} from './wasm-loader-clang-format-config-migrate.service';

describe('WasmLoaderClangFormatConfigMigrateService', () => {
  let service: WasmLoaderClangFormatConfigMigrateService;

  beforeEach(() => {
    TestBed.configureTestingModule({});
    service = TestBed.inject(WasmLoaderClangFormatConfigMigrateService);
  });

  it('should be created', () => {
    expect(service).toBeTruthy();
  });

});
