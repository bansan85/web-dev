import { importProvidersFrom } from '@angular/core';
import { TestBed } from '@angular/core/testing';
import { LoaderCircle, LucideAngularModule, Settings, X } from 'lucide-angular';

import { AppComponent } from './app.component';

describe('AppComponent', () => {
  beforeEach(async () => {
    await TestBed.configureTestingModule({
      imports: [AppComponent],
      providers: [
        importProvidersFrom(
          LucideAngularModule.pick({ Settings, LoaderCircle, X })
        ),
      ],
    }).compileComponents();
  });

  it('should create the app', () => {
    const fixture = TestBed.createComponent(AppComponent);
    const app = fixture.componentInstance;
    expect(app).toBeTruthy();
  });
});
