import { CommonModule } from '@angular/common';
import {
  ChangeDetectionStrategy,
  Component,
  contentChild,
  signal,
  TemplateRef,
} from '@angular/core';

@Component({
  selector: 'app-tab-item',
  imports: [CommonModule],
  templateUrl: './tab-item.html',
  styleUrl: './tab-item.css',
  changeDetection: ChangeDetectionStrategy.OnPush,
})
export class TabItem {
  readonly header = contentChild(TemplateRef);
  readonly activate = signal(true);
}
