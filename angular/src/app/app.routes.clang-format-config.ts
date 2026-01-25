import { Routes } from '@angular/router';

import { ClangFormatConfigMigrateComponent } from './apps/clang-format-config-migrate.component';

export const routes: Routes = [
  { path: '', component: ClangFormatConfigMigrateComponent },
  { path: '**', redirectTo: '' }
];
