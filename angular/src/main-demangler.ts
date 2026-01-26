import { bootstrapApplication } from '@angular/platform-browser';

import { appConfig } from './app/app.config';
import { AppDemanglerComponent } from './app/apps/demangler.component';

bootstrapApplication(AppDemanglerComponent, appConfig).catch((err: unknown) =>
  { console.error(err); }
);
