import { ComponentFixture, TestBed } from '@angular/core/testing';

import { ClangFormatConfigMigrateComponent } from './clang-format-config-migrate.component';

describe('ClangFormatConfigMigrateComponent', () => {
  let component: ClangFormatConfigMigrateComponent;
  let fixture: ComponentFixture<ClangFormatConfigMigrateComponent>;

  beforeEach(async () => {
    await TestBed.configureTestingModule({
      imports: [ClangFormatConfigMigrateComponent]
    })
    .compileComponents();

    fixture = TestBed.createComponent(ClangFormatConfigMigrateComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
