
import { Component, computed, inject } from '@angular/core';
import {
  NavigationEnd,
  Router,
  RouterLink,
  RouterLinkActive,
  RouterOutlet,
} from '@angular/router';
import { LogoDemanglerInlineComponent } from './img/logo-demangler-inline.component';
import { LogoFormatterInlineComponent } from './img/logo-formatter-inline.component';
import { LogoNamingStyleInlineComponent } from './img/logo-naming-style-inline.component';
import { LogoLightenInlineComponent } from './img/logo-lighten-inline.component';
import { GithubMarkInlineComponent } from './img/github-mark-inline.component';
import { LogoClangFormatConfigMigrateInlineComponent } from './img/logo-clang-format-config-migrate-inline.component'
import { filter, map } from 'rxjs';
import { toSignal } from '@angular/core/rxjs-interop';

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
  styleUrl: './app.component.css'
})
export class AppComponent {
  private router = inject(Router);

  private readonly url = toSignal(
    this.router.events.pipe(
      filter((event) => event instanceof NavigationEnd),
      map(() => this.router.url)
    ),
    { initialValue: this.router.url }
  );

  protected readonly isHomeRoute = computed(() => this.url() === '/');
}

