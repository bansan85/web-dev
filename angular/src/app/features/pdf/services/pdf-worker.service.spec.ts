import { TestBed } from '@angular/core/testing';

import { PdfWorkerService } from './pdf-worker.service';

describe('PdfWorkerService', () => {
  let service: PdfWorkerService;

  beforeEach(() => {
    TestBed.configureTestingModule({});
    service = TestBed.inject(PdfWorkerService);
  });

  it('should be created', () => {
    expect(service).toBeTruthy();
  });
});
