import { ComponentFixture, TestBed } from '@angular/core/testing';

import { AppNamingStyleComponent } from './naming-style.component';
import { importProvidersFrom } from '@angular/core';
import { LucideAngularModule, Copy, RotateCcw, NotebookPen } from 'lucide-angular';

describe('NamingStyleComponent', () => {
  let component: AppNamingStyleComponent;
  let fixture: ComponentFixture<AppNamingStyleComponent>;

  beforeEach(async () => {
    await TestBed.configureTestingModule({
      imports: [AppNamingStyleComponent],
      providers: [
        importProvidersFrom(
          LucideAngularModule.pick({ Copy, RotateCcw, NotebookPen })
        ),
      ],
    })
      .compileComponents();

    fixture = TestBed.createComponent(AppNamingStyleComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
