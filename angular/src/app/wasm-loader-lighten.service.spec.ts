import { TestBed } from '@angular/core/testing';

import { WasmLoaderLightenService } from './wasm-loader-lighten.service';
import { EmbindModule as LightenModule } from '../assets/web_lighten.js';

describe('WasmLoaderLightenService', () => {
  let service: WasmLoaderLightenService;

  beforeEach(() => {
    TestBed.configureTestingModule({});
    service = TestBed.inject(WasmLoaderLightenService);
  });

  it('should be created', () => {
    expect(service).toBeTruthy();
  });

  it('check lighten', async () => {
    const lighten: LightenModule = await service.wasm()!;

    expect(lighten.web_lighten_number('1')).toEqual('1');
    expect(lighten.web_lighten_number('1.')).toEqual('1');

    expect(lighten.web_lighten_number('1.1012')).toEqual('1.1012');
    expect(lighten.web_lighten_number('1.100195')).toEqual('1.100195');
    expect(lighten.web_lighten_number('1.1000175')).toEqual('1.1000175');
    expect(lighten.web_lighten_number('1.10000165')).toEqual('1.1');
    expect(lighten.web_lighten_number('1.100000116')).toEqual('1.1');
    expect(lighten.web_lighten_number('1.1000000135')).toEqual('1.1');

    expect(lighten.web_lighten_number('110.12')).toEqual('110.12');
    expect(lighten.web_lighten_number('110.0195')).toEqual('110.0195');
    expect(lighten.web_lighten_number('1100.0175')).toEqual('1100.0175');
    expect(lighten.web_lighten_number('11000.0165')).toEqual('11000');
    expect(lighten.web_lighten_number('11000.00116')).toEqual('11000');
    expect(lighten.web_lighten_number('11000.000135')).toEqual('11000');

    expect(lighten.web_lighten_number('11012')).toEqual('11012');
    expect(lighten.web_lighten_number('110019.5')).toEqual('110019.5');
    expect(lighten.web_lighten_number('1100017.5')).toEqual('1100017.5');
    expect(lighten.web_lighten_number('11000016.5')).toEqual('11000000');
    expect(lighten.web_lighten_number('110000011.6')).toEqual('110000000');
    expect(lighten.web_lighten_number('1100000013.5')).toEqual('1100000000');

    expect(lighten.web_lighten_number('999999')).toEqual('1000000');
    expect(lighten.web_lighten_number('50000001')).toEqual('50000000');

    expect(lighten.web_lighten_number('1.1912')).toEqual('1.1912');
    expect(lighten.web_lighten_number('1.199195')).toEqual('1.199195');
    expect(lighten.web_lighten_number('1.1999175')).toEqual('1.1999175');
    expect(lighten.web_lighten_number('1.19999165')).toEqual('1.2');
    expect(lighten.web_lighten_number('1.199999116')).toEqual('1.2');
    expect(lighten.web_lighten_number('1.1999999135')).toEqual('1.2')

    expect(lighten.web_lighten_number('11912')).toEqual('11912');
    expect(lighten.web_lighten_number('119919.5')).toEqual('119919.5');
    expect(lighten.web_lighten_number('1199917.5')).toEqual('1199917.5');
    expect(lighten.web_lighten_number('11999916.5')).toEqual('12000000');
    expect(lighten.web_lighten_number('119999911.6')).toEqual('120000000');
    expect(lighten.web_lighten_number('1199999913.5')).toEqual('1200000000');

    expect(lighten.web_lighten_number('-1')).toEqual('-1');
    expect(lighten.web_lighten_number('-1.')).toEqual('-1');

    expect(lighten.web_lighten_number('-1.1012')).toEqual('-1.1012');
    expect(lighten.web_lighten_number('-1.100195')).toEqual('-1.100195');
    expect(lighten.web_lighten_number('-1.1000175')).toEqual('-1.1000175');
    expect(lighten.web_lighten_number('-1.10000165')).toEqual('-1.1');
    expect(lighten.web_lighten_number('-1.100000116')).toEqual('-1.1');
    expect(lighten.web_lighten_number('-1.1000000135')).toEqual('-1.1');

    expect(lighten.web_lighten_number('-110.12')).toEqual('-110.12');
    expect(lighten.web_lighten_number('-110.0195')).toEqual('-110.0195');
    expect(lighten.web_lighten_number('-1100.0175')).toEqual('-1100.0175');
    expect(lighten.web_lighten_number('-11000.0165')).toEqual('-11000');
    expect(lighten.web_lighten_number('-11000.00116')).toEqual('-11000');
    expect(lighten.web_lighten_number('-11000.000135')).toEqual('-11000');

    expect(lighten.web_lighten_number('-11012')).toEqual('-11012');
    expect(lighten.web_lighten_number('-110019.5')).toEqual('-110019.5');
    expect(lighten.web_lighten_number('-1100017.5')).toEqual('-1100017.5');
    expect(lighten.web_lighten_number('-11000016.5')).toEqual('-11000000');
    expect(lighten.web_lighten_number('-110000011.6')).toEqual('-110000000');
    expect(lighten.web_lighten_number('-1100000013.5')).toEqual('-1100000000');

    expect(lighten.web_lighten_number('-999999')).toEqual('-1000000');
    expect(lighten.web_lighten_number('-50000001')).toEqual('-50000000');

    expect(lighten.web_lighten_number('-1.1912')).toEqual('-1.1912');
    expect(lighten.web_lighten_number('-1.199195')).toEqual('-1.199195');
    expect(lighten.web_lighten_number('-1.1999175')).toEqual('-1.1999175');
    expect(lighten.web_lighten_number('-1.19999165')).toEqual('-1.2');
    expect(lighten.web_lighten_number('-1.199999116')).toEqual('-1.2');
    expect(lighten.web_lighten_number('-1.1999999135')).toEqual('-1.2')

    expect(lighten.web_lighten_number('-11912')).toEqual('-11912');
    expect(lighten.web_lighten_number('-119919.5')).toEqual('-119919.5');
    expect(lighten.web_lighten_number('-1199917.5')).toEqual('-1199917.5');
    expect(lighten.web_lighten_number('-11999916.5')).toEqual('-12000000');
    expect(lighten.web_lighten_number('-119999911.6')).toEqual('-120000000');
    expect(lighten.web_lighten_number('-1199999913.5')).toEqual('-1200000000');
  });
});