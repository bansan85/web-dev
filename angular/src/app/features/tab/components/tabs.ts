import { CommonModule } from '@angular/common';
import {
  AfterContentInit,
  ChangeDetectionStrategy,
  Component,
  contentChildren,
} from '@angular/core';

import { TabItem } from './tab-item';

@Component({
  selector: 'app-tabs',
  imports: [CommonModule],
  templateUrl: './tabs.html',
  styleUrl: './tabs.css',
  changeDetection: ChangeDetectionStrategy.OnPush,
})
export class Tabs implements AfterContentInit {
  protected readonly tabs = contentChildren(TabItem);

  protected activeComponent = 0;

  ngAfterContentInit() {
    this.activateTab(0);
  }

  activateTab(index: number) {
    this.activeComponent = index;
    for (const [i, tab] of this.tabs().entries()) {
      tab.activate.set(i === index);
    }
  }
}
