import { CommonModule } from '@angular/common';
import {
  ChangeDetectionStrategy,
  Component,
  input,
  signal,
} from '@angular/core';

@Component({
  selector: 'app-tab-item',
  imports: [CommonModule],
  templateUrl: './tab-item.html',
  styleUrl: './tab-item.css',
  changeDetection: ChangeDetectionStrategy.OnPush,
})
export class TabItem {
  readonly tabName = input<string>('default');
  readonly activate = signal(true);
}
