import { importProvidersFrom } from '@angular/core';
import { ComponentFixture, TestBed } from '@angular/core/testing';
import { LoaderCircle,LucideAngularModule, Settings, X } from 'lucide-angular';

import { AppFormatterComponent } from './formatter.component';

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
