import { Routes } from '@angular/router';

import { AppLightenComponent } from './apps/lighten.component';

export const routes: Routes = [
  { path: '', component: AppLightenComponent },
  { path: '**', redirectTo: '' }
];
