import { TestBed } from '@angular/core/testing';

import {
  WasmLoaderFormatterService,
  FormatterModule,
  FormatStyle,
} from './wasm-loader-formatter.service';

describe('WasmLoaderFormatterService', () => {
  let service: WasmLoaderFormatterService;

  beforeEach(() => {
    TestBed.configureTestingModule({});
    service = TestBed.inject(WasmLoaderFormatterService);
  });

  it('should be created', () => {
    expect(service).toBeTruthy();
  });

  it('check formatter', async () => {
    const formatter: FormatterModule = await service.wasm();

    const llvmStyle: FormatStyle = formatter.getLLVMStyle();
    expect(formatter.formatter('int main(){int a; int b;}', llvmStyle))
      .toEqual(`int main() {
  int a;
  int b;
}`);
  });
});
