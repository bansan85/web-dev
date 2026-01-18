import { ChangeDetectionStrategy, Component } from '@angular/core';

@Component({
  selector: 'app-logo-naming-style-inline',
  standalone: true,
  templateUrl: '../../assets/img/logo-naming-style.svg',
  changeDetection: ChangeDetectionStrategy.OnPush,
})
export class LogoNamingStyleInlineComponent { }
