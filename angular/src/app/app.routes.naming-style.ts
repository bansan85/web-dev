import { Routes } from '@angular/router';

import { AppNamingStyleComponent } from './apps/naming-style.component';

export const routes: Routes = [
  { path: '', component: AppNamingStyleComponent },
  { path: '**', redirectTo: '' }
];
