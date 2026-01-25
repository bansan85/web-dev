import { bootstrapApplication } from '@angular/platform-browser';

import { appConfig } from './app/app.config';
import { AppFormatterComponent } from './app/apps/formatter.component';

bootstrapApplication(AppFormatterComponent, appConfig).catch((err: unknown) =>
  { console.error(err); }
);
