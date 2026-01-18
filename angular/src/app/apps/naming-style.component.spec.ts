import { importProvidersFrom } from '@angular/core';
import { ComponentFixture, TestBed } from '@angular/core/testing';
import { Copy, LucideAngularModule, NotebookPen,RotateCcw } from 'lucide-angular';

import { AppNamingStyleComponent } from './naming-style.component';

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
