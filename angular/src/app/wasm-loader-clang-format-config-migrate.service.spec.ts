import { TestBed } from '@angular/core/testing';

import { EmbindModule as ClangFormatConfigMigrateModule } from '../assets/web_clang_format_config_migrate.js';
import { WasmLoaderClangFormatConfigMigrateService } from './wasm-loader-clang-format-config-migrate.service';

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
