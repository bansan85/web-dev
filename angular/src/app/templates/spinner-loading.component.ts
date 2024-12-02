import { Component, Input } from '@angular/core';
import { LucideAngularModule } from 'lucide-angular';

@Component({
  selector: 'app-spinner-loading',
  standalone: true,
  imports: [LucideAngularModule],
  templateUrl: './spinner-loading.component.html',
  styleUrl: './spinner-loading.component.css',
})
export class SpinnerLoadingComponent {
  @Input({ required: true }) size!: number;
  @Input({ required: true }) title!: string;
}
