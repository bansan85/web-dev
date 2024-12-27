import { Routes } from '@angular/router';
import { AppDemanglerComponent } from './apps/demangler.component';
import { AppFormatterComponent } from './apps/formatter.component';
import { AppNamingStyleComponent } from './apps/naming-style.component';
import { AppLightenComponent } from './apps/lighten.component';

export const routes: Routes = [
  { path: 'demangler', component: AppDemanglerComponent },
  { path: 'formatter', component: AppFormatterComponent },
  { path: 'naming-style', component: AppNamingStyleComponent },
  { path: 'lighten', component: AppLightenComponent },
];
