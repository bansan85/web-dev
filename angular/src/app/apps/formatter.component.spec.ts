import { ComponentFixture, TestBed } from '@angular/core/testing';

import { AppFormatterComponent } from './formatter.component';
import { importProvidersFrom } from '@angular/core';
import { LucideAngularModule, Settings, X, LoaderCircle } from 'lucide-angular';

describe('FormatterComponent', () => {
  let component: AppFormatterComponent;
  let fixture: ComponentFixture<AppFormatterComponent>;

  beforeEach(async () => {
    await TestBed.configureTestingModule({
      imports: [AppFormatterComponent],
      providers: [
        importProvidersFrom(
          LucideAngularModule.pick({ Settings, X, LoaderCircle })
        ),
      ],
    }).compileComponents();

    fixture = TestBed.createComponent(AppFormatterComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
