import { ChangeDetectionStrategy, Component } from '@angular/core';

@Component({
  selector: 'app-logo-lighten-inline',
  standalone: true,
  templateUrl: '../../assets/img/logo-lighten.svg',
  changeDetection: ChangeDetectionStrategy.OnPush,
})
export class LogoLightenInlineComponent { }
