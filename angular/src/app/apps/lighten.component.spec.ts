import { importProvidersFrom } from '@angular/core';
import { ComponentFixture, TestBed } from '@angular/core/testing';
import { LucideAngularModule, Settings, X } from 'lucide-angular';

import { AppLightenComponent } from './lighten.component';

describe('LightenComponent', () => {
  let component: AppLightenComponent;
  let fixture: ComponentFixture<AppLightenComponent>;

  beforeEach(async () => {
    await TestBed.configureTestingModule({
      imports: [AppLightenComponent],
      providers: [
        importProvidersFrom(LucideAngularModule.pick({ Settings, X })),
      ],
    }).compileComponents();

    fixture = TestBed.createComponent(AppLightenComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
