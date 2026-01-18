import { ChangeDetectionStrategy, Component } from '@angular/core';

@Component({
  selector: 'app-logo-demangler-inline',
  standalone: true,
  templateUrl: '../../assets/img/logo-demangler.svg',
  changeDetection: ChangeDetectionStrategy.OnPush,
})
export class LogoDemanglerInlineComponent {}
