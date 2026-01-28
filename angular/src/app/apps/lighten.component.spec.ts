import { importProvidersFrom } from '@angular/core';
import { ComponentFixture, TestBed } from '@angular/core/testing';
import { By } from '@angular/platform-browser';
import { LucideAngularModule, Settings, X } from 'lucide-angular';

import { AppLightenComponent } from './lighten.component';

function sleep(ms: number): Promise<void> {
  return new Promise((resolve) => {
    setTimeout(resolve, ms);
  });
}

class Page {
  constructor(
    private readonly fixture: ComponentFixture<AppLightenComponent>
  ) { }

  get textareaInput() {
    return this.fixture.debugElement.query(By.css('.textareas .textarea:first-child textarea'))
      .nativeElement as HTMLTextAreaElement;
  }

  get textareaOutput() {
    return this.fixture.debugElement.query(By.css('.textareas .textarea:nth-child(2) textarea'))
      .nativeElement as HTMLTextAreaElement;
  }

  get settingsImage() {
    return this.fixture.debugElement.query(By.css('lucide-icon[name="settings"]'))
      .nativeElement as HTMLElement;
  }

  get settingsCountInput() {
    return this.fixture.debugElement.query(By.css('app-dialog-popup input[name="count"]'))
      .nativeElement as HTMLInputElement;
  }

  isDialogClosed() {
    return this.fixture.debugElement.query(
      By.css('app-dialog-popup dialog:not(.open)')
    )
      .nativeElement !== undefined;
  }

  isDialogOpened() {
    return this.fixture.debugElement.query(
      By.css('app-dialog-popup dialog.open')
    )
      .nativeElement !== undefined;
  }
}

describe('LightenComponent', () => {
  let component: AppLightenComponent;
  let fixture: ComponentFixture<AppLightenComponent>;
  let page: Page;

  beforeEach(async () => {
    await TestBed.configureTestingModule({
      imports: [AppLightenComponent],
      providers: [
        importProvidersFrom(LucideAngularModule.pick({ Settings, X })),
      ],
    }).compileComponents();

    fixture = TestBed.createComponent(AppLightenComponent);
    component = fixture.componentInstance;
    page = new Page(fixture);

    await fixture.whenStable();
  });

  it('should lighten numbers', async () => {
    expect(component).toBeTruthy();
    expect(page.textareaInput).toBeTruthy();
    expect(page.textareaOutput).toBeTruthy();
    expect(page.settingsImage).toBeTruthy();

    await component.loadWasmLightenModule();

    fixture.detectChanges();

    const { textareaInput, textareaOutput, settingsImage, settingsCountInput } = page;
    textareaInput.value = "[1.2323, 3200000001111, 3200.000001111]";
    textareaInput.dispatchEvent(new Event('input'));
    await fixture.whenStable();
    expect(textareaOutput.value).toBe(`[
  1.2323,
  3200000000000,
  3200
]`);

    expect(page.isDialogClosed()).toBe(true);

    settingsImage.click();

    await fixture.whenStable();
    await sleep(0);
    await fixture.whenStable();

    expect(page.isDialogOpened()).toBe(true);

    expect(settingsCountInput.value).toBe("4");
    settingsCountInput.value = "8";
    settingsCountInput.dispatchEvent(new Event('input'));
    await fixture.whenStable();
    expect(textareaOutput.value).toBe(`[
  1.2323,
  3200000001111,
  3200.000001111
]`);
  });
});
