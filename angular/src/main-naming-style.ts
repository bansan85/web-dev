import { bootstrapApplication } from '@angular/platform-browser';

import { appConfig } from './app/app.config';
import { AppNamingStyleComponent } from './app/apps/naming-style.component';

bootstrapApplication(AppNamingStyleComponent, appConfig).catch((err: unknown) =>
  { console.error(err); }
);
