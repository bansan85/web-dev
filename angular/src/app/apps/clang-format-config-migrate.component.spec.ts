import { ComponentFixture, TestBed } from '@angular/core/testing';
import { importProvidersFrom } from '@angular/core';
import { LucideAngularModule, Settings, X, LoaderCircle } from 'lucide-angular';

import { ClangFormatConfigMigrateComponent } from './clang-format-config-migrate.component';

describe('ClangFormatConfigMigrateComponent', () => {
  let component: ClangFormatConfigMigrateComponent;
  let fixture: ComponentFixture<ClangFormatConfigMigrateComponent>;

  beforeEach(async () => {
    await TestBed.configureTestingModule({
      imports: [ClangFormatConfigMigrateComponent],
      providers: [
        importProvidersFrom(LucideAngularModule.pick({ Settings, X, LoaderCircle })),
      ],
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
