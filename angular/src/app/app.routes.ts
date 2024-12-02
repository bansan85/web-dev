import { Routes } from '@angular/router';
import { AppDemanglerComponent } from './apps/demangler.component';
import { AppFormatterComponent } from './apps/formatter.component';

export const routes: Routes = [
  { path: 'demangler', component: AppDemanglerComponent },
  { path: 'formatter', component: AppFormatterComponent },
];
