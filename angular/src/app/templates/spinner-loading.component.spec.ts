import { importProvidersFrom } from '@angular/core';
import { ComponentFixture, TestBed } from '@angular/core/testing';
import { LoaderCircle,LucideAngularModule } from 'lucide-angular';

import { SpinnerLoadingComponent } from './spinner-loading.component';

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
    fixture.componentRef.setInput('size', 150);
    fixture.componentRef.setInput('title', 'title');
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
