import { importProvidersFrom } from '@angular/core';
import { ComponentFixture, TestBed } from '@angular/core/testing';
import { LucideAngularModule, X } from 'lucide-angular';

import { DialogPopupComponent } from './dialog-popup.component';

describe('DialogPopupComponent', () => {
  let component: DialogPopupComponent;
  let fixture: ComponentFixture<DialogPopupComponent>;

  beforeEach(async () => {
    await TestBed.configureTestingModule({
      imports: [DialogPopupComponent],
      providers: [importProvidersFrom(LucideAngularModule.pick({ X }))],
    }).compileComponents();

    fixture = TestBed.createComponent(DialogPopupComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
