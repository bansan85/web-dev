import { ComponentFixture, TestBed } from '@angular/core/testing';

import { ClangFormatReadabilityIdentifierNamingComponent } from './clang-format-readability-identifier-naming.component';

describe('ClangFormatReadabilityIdentifierNamingComponent', () => {
  let component: ClangFormatReadabilityIdentifierNamingComponent;
  let fixture: ComponentFixture<ClangFormatReadabilityIdentifierNamingComponent>;

  beforeEach(async () => {
    await TestBed.configureTestingModule({
      imports: [ClangFormatReadabilityIdentifierNamingComponent]
    })
    .compileComponents();

    fixture = TestBed.createComponent(ClangFormatReadabilityIdentifierNamingComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
