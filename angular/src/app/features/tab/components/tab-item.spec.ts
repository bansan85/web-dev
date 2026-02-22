import { ComponentFixture, TestBed } from '@angular/core/testing';
import { beforeEach, describe, expect, it } from 'vitest';

import { TabItem } from './tab-item';

describe('TabItem', () => {
  let component: TabItem;
  let fixture: ComponentFixture<TabItem>;

  beforeEach(async () => {
    await TestBed.configureTestingModule({
      imports: [TabItem],
    }).compileComponents();

    fixture = TestBed.createComponent(TabItem);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
