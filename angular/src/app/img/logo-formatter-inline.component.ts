import { ChangeDetectionStrategy, Component } from '@angular/core';

@Component({
  selector: 'app-logo-formatter-inline',
  standalone: true,
  templateUrl: '../../assets/img/logo-formatter.svg',
  changeDetection: ChangeDetectionStrategy.OnPush,
})
export class LogoFormatterInlineComponent {}
