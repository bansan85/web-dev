import { bootstrapApplication } from '@angular/platform-browser';

import { appConfig } from './app/app.config';
import { ClangFormatConfigMigrateComponent } from './app/apps/clang-format-config-migrate.component';

bootstrapApplication(ClangFormatConfigMigrateComponent, appConfig).catch((err: unknown) =>
  { console.error(err); }
);
