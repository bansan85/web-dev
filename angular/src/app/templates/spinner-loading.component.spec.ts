import { ComponentFixture, TestBed } from '@angular/core/testing';
import { LucideAngularModule, LoaderCircle } from 'lucide-angular';

import { SpinnerLoadingComponent } from './spinner-loading.component';
import { importProvidersFrom } from '@angular/core';

describe('SpinnerLoadingComponent', () => {
  let component: SpinnerLoadingComponent;
  let fixture: ComponentFixture<SpinnerLoadingComponent>;

  beforeEach(async () => {
    await TestBed.configureTestingModule({
      imports: [SpinnerLoadingComponent],
      providers: [
        importProvidersFrom(LucideAngularModule.pick({ LoaderCircle })),
      ],
    }).compileComponents();

    fixture = TestBed.createComponent(SpinnerLoadingComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
