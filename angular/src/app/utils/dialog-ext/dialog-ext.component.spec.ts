import { ComponentFixture, TestBed } from '@angular/core/testing';

import { DialogExtComponent } from './dialog-ext.component';
import { importProvidersFrom } from '@angular/core';
import { LucideAngularModule, X } from 'lucide-angular';

describe('DialogExtComponent', () => {
  let component: DialogExtComponent;
  let fixture: ComponentFixture<DialogExtComponent>;

  beforeEach(async () => {
    await TestBed.configureTestingModule({
      imports: [DialogExtComponent],
      providers: [importProvidersFrom(LucideAngularModule.pick({ X }))],
    }).compileComponents();

    fixture = TestBed.createComponent(DialogExtComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
