import { Routes } from '@angular/router';

import { ClangFormatConfigMigrateComponent } from './apps/clang-format-config-migrate.component';
import { AppDemanglerComponent } from './apps/demangler.component';
import { AppFormatterComponent } from './apps/formatter.component';
import { AppLightenComponent } from './apps/lighten.component';
import { AppNamingStyleComponent } from './apps/naming-style.component';

export const routes: Routes = [
  { path: 'demangler', component: AppDemanglerComponent },
  { path: 'formatter', component: AppFormatterComponent },
  { path: 'naming-style', component: AppNamingStyleComponent },
  { path: 'lighten', component: AppLightenComponent },
  { path: 'clang-format-config-migrate', component: ClangFormatConfigMigrateComponent },
];
