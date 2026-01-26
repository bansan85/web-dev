import { Routes } from '@angular/router';

import { AppDemanglerComponent } from './apps/demangler.component';

export const routes: Routes = [
  { path: '', component: AppDemanglerComponent },
  { path: '**', redirectTo: '' }
];
