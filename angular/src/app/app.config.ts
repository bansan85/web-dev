import {
  ApplicationConfig,
  provideZonelessChangeDetection,
  importProvidersFrom,
} from '@angular/core';
import { provideRouter } from '@angular/router';
import { LucideAngularModule, Settings, LoaderCircle, X, Copy, RotateCcw, NotebookPen } from 'lucide-angular';

import { routes } from './app.routes';

export const appConfig: ApplicationConfig = {
  providers: [
    provideZonelessChangeDetection(),
    provideRouter(routes),
    importProvidersFrom(
      LucideAngularModule.pick({ Settings, LoaderCircle, X, Copy, RotateCcw, NotebookPen })
    ),
  ],
};
