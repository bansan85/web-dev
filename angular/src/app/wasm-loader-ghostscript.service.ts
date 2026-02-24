import { Injectable, signal } from '@angular/core';

@Injectable({
  providedIn: 'root',
})
export class WasmLoaderGhostscriptService {
  private readonly loading = signal(false);
  readonly isLoading = this.loading.asReadonly();

  private preload_promise?: Promise<void>;

  preload(): Promise<void> {
    if (this.preload_promise) {
      return this.preload_promise;
    }

    this.loading.set(true);

    this.preload_promise = fetch('gs.wasm', {
      cache: 'force-cache',
    })
      .then((response) => {
        if (!response.ok) {
          throw new Error('Failed to preload gs.wasm');
        }
        return response.arrayBuffer();
      })
      .then(() => {
        this.loading.set(false);
      })
      .catch((error) => {
        this.loading.set(false);
        throw error;
      });

    return this.preload_promise;
  }
}
