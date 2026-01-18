import { ChangeDetectionStrategy, Component, input } from '@angular/core';
import { LucideAngularModule } from 'lucide-angular';

@Component({
  selector: 'app-spinner-loading',
  imports: [LucideAngularModule],
  templateUrl: './spinner-loading.component.html',
  styleUrl: './spinner-loading.component.css',
  changeDetection: ChangeDetectionStrategy.OnPush,
})
export class SpinnerLoadingComponent {
  readonly size = input.required<number>();
  readonly title = input.required<string>();
}
