import { bootstrapApplication } from '@angular/platform-browser';

import { appConfig } from './app/app.config';
import { AppLightenComponent } from './app/apps/lighten.component';

bootstrapApplication(AppLightenComponent, appConfig).catch((err: unknown) =>
  { console.error(err); }
);
