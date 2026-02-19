import { Routes } from '@angular/router';

import { MainPdfComponent } from './apps/main-pdf.component';

export const routes: Routes = [
  { path: '', component: MainPdfComponent },
  { path: '**', redirectTo: '' }
];
