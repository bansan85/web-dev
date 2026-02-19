import { bootstrapApplication } from '@angular/platform-browser';

import { appConfig } from './app/app.config';
import { MainPdfComponent } from './app/apps/main-pdf.component';

bootstrapApplication(MainPdfComponent, appConfig).catch((err: unknown) => { console.error(err); }
);
