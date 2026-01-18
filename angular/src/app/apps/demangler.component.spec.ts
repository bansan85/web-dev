import { importProvidersFrom } from '@angular/core';
import { TestBed } from '@angular/core/testing';
import { By } from '@angular/platform-browser';
import { LoaderCircle, LucideAngularModule, Settings, X } from 'lucide-angular';

import { AppDemanglerComponent } from './demangler.component';

function sleep(ms: number): Promise<void> {
  return new Promise((resolve) => {
    setTimeout(resolve, ms);
  });
}

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
    expect(fixture.debugElement.query(
      By.css('app-spinner-loading')
    )).toBeNull();

    await app.loadWasmDemanglerModule();
    await app.loadWasmFormatterModule();

    fixture.detectChanges();

    expect(fixture.debugElement.query(
      By.css('app-spinner-loading')
    )).toBeNull();

    // Test open clang-format dialog
    // eslint-disable-next-line @typescript-eslint/no-unnecessary-type-assertion
    const settingsImage = document.querySelector(
      'lucide-icon[name="settings"]'
    )! as HTMLElement;
    expect(settingsImage).toBeDefined();

    expect(fixture.debugElement.query(
      By.css('app-dialog-popup dialog:not(.open)')
    )).toBeTruthy();

    settingsImage.click();

    await fixture.whenStable();
    await sleep(0);
    await fixture.whenStable();

    expect(fixture.debugElement.query(
      By.css('app-dialog-popup dialog.open')
    )).toBeTruthy();

    // eslint-disable-next-line @typescript-eslint/no-unnecessary-type-assertion
    const checkedClangFormat = document.querySelector(
      'input[name="enableClangFormat"]'
    )! as HTMLElement;
    checkedClangFormat.click();
    fixture.detectChanges();
    await fixture.whenStable();

    expect(app.formatStyle).toBeDefined();
  });
});
