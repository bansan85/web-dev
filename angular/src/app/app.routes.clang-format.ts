import { Routes } from '@angular/router';

import { AppFormatterComponent } from './apps/formatter.component';

export const routes: Routes = [
  { path: '', component: AppFormatterComponent },
  { path: '**', redirectTo: '' }
];
