import {
  ApplicationConfig,
  importProvidersFrom,
  provideZonelessChangeDetection,
} from '@angular/core';
import { provideRouter } from '@angular/router';
import { Copy, LoaderCircle, LucideAngularModule, NotebookPen,RotateCcw, Settings, X } from 'lucide-angular';

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
