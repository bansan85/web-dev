import { TestBed } from '@angular/core/testing';
import { AppDemanglerComponent } from './demangler.component';
import { importProvidersFrom } from '@angular/core';
import { LoaderCircle, LucideAngularModule, Settings, X } from 'lucide-angular';

describe('AppDemanglerComponent', () => {
  beforeEach(async () => {
    await TestBed.configureTestingModule({
      imports: [AppDemanglerComponent],
      providers: [
        importProvidersFrom(LucideAngularModule.pick({ Settings, X, LoaderCircle })),
      ],
    }).compileComponents();
  });

  it('should create the app', () => {
    const fixture = TestBed.createComponent(AppDemanglerComponent);
    const app = fixture.componentInstance;
    expect(app).toBeTruthy();
  });

  it('configuration of clang-format', async () => {
    const fixture = TestBed.createComponent(AppDemanglerComponent);
    const app = fixture.componentInstance;

    // Test the loading of wasm for formatStyle
    expect(app.formatStyle).toBeUndefined();

    await app.loadWasmDemanglerModule();
    await app.loadWasmFormatterModule();

    fixture.detectChanges();

    // Test open clang-format dialog
    const settingsImage = document.querySelector(
      'lucide-icon[name="settings"]'
    ) as HTMLElement;
    expect(settingsImage).toBeDefined();

    settingsImage!.click();

    await fixture.whenStable();

    const checkedClangFormat = document.querySelector(
      'input[name="enableClangFormat"]'
    ) as HTMLElement;
    checkedClangFormat.click();
    fixture.detectChanges();
    await fixture.whenStable();

    expect(app.formatStyle).toBeDefined();
  });
});
