import { ChangeDetectionStrategy, Component } from '@angular/core';

@Component({
  selector: 'app-logo-main-pdf-inline',
  standalone: true,
  templateUrl: '../../assets/img/logo-main-pdf.svg',
  changeDetection: ChangeDetectionStrategy.OnPush,
})
export class LogoMainPdfInlineComponent { }
