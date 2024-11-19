import { TestBed } from '@angular/core/testing';

import { WasmLoaderDemanglerService } from './wasm-loader-demangler.service';
import { EmbindModule as DemanglerModule } from '../assets/web_demangler.js';

describe('WasmLoaderDemanglerService', () => {
  let service: WasmLoaderDemanglerService;

  beforeEach(() => {
    TestBed.configureTestingModule({});
    service = TestBed.inject(WasmLoaderDemanglerService);
  });

  it('should be created', () => {
    expect(service).toBeTruthy();
  });

  it('check demangler', async () => {
    let demangler: DemanglerModule = await service.wasm()!;

    expect(demangler.web_demangle('_ZTI1Y')).toEqual('typeinfo for Y');
    expect(demangler.web_demangle('_ZN1BD1Ev')).toEqual('B::~B()');
    expect(demangler.web_demangle('??1A@@UAE@XZ')).toEqual(
      'public: virtual __thiscall A::~A(void)'
    );
    expect(
      demangler.web_demangle('_ZNK8KxVectorI16KxfArcFileRecordjEixEj')
    ).toEqual(
      'KxVector<KxfArcFileRecord, unsigned int>::operator[](unsigned int) const'
    );

    // Angular returns Itanium without _Z prefix.
    expect(
      demangler.web_demangle(
        'NSt3__26vectorIN5clang7tooling12IncludeStyle15IncludeCategoryENS_9allocatorIS4_EEEE'
      )
    ).toEqual(
      'std::__2::vector<clang::tooling::IncludeStyle::IncludeCategory, std::__2::allocator<clang::tooling::IncludeStyle::IncludeCategory>>'
    );
  });
});
