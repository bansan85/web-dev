import { importProvidersFrom } from '@angular/core';
import { ComponentFixture, TestBed } from '@angular/core/testing';
import { Copy, Import, LucideAngularModule, NotebookPen, RefreshCcw, X } from 'lucide-angular';

import { AppNamingStyleComponent } from './naming-style.component';

describe('NamingStyleComponent', () => {
  let component: AppNamingStyleComponent;
  let fixture: ComponentFixture<AppNamingStyleComponent>;

  beforeEach(async () => {
    await TestBed.configureTestingModule({
      imports: [AppNamingStyleComponent],
      providers: [
        importProvidersFrom(
          LucideAngularModule.pick({ Copy, RefreshCcw, NotebookPen, Import, X })
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
