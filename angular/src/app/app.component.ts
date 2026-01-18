
import { ChangeDetectionStrategy, Component, computed, inject } from '@angular/core';
import { toSignal } from '@angular/core/rxjs-interop';
import {
  NavigationEnd,
  Router,
  RouterLink,
  RouterLinkActive,
  RouterOutlet,
} from '@angular/router';
import { filter, map } from 'rxjs';

import { GithubMarkInlineComponent } from './img/github-mark-inline.component';
import { LogoClangFormatConfigMigrateInlineComponent } from './img/logo-clang-format-config-migrate-inline.component'
import { LogoDemanglerInlineComponent } from './img/logo-demangler-inline.component';
import { LogoFormatterInlineComponent } from './img/logo-formatter-inline.component';
import { LogoLightenInlineComponent } from './img/logo-lighten-inline.component';
import { LogoNamingStyleInlineComponent } from './img/logo-naming-style-inline.component';

@Component({
  selector: 'app-root',
  imports: [
    RouterOutlet,
    RouterLink,
    RouterLinkActive,
    LogoDemanglerInlineComponent,
    LogoFormatterInlineComponent,
    LogoNamingStyleInlineComponent,
    LogoLightenInlineComponent,
    GithubMarkInlineComponent,
    LogoClangFormatConfigMigrateInlineComponent
  ],
  templateUrl: './app.component.html',
  styleUrl: './app.component.css',
  changeDetection: ChangeDetectionStrategy.OnPush,
})
export class AppComponent {
  private readonly router = inject(Router);

  private readonly url = toSignal(
    this.router.events.pipe(
      filter((event) => event instanceof NavigationEnd),
      map(() => this.router.url)
    ),
    { initialValue: this.router.url }
  );

  protected readonly isHomeRoute = computed(() => this.url() === '/');
}

