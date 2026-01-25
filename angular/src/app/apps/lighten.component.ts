import { ChangeDetectionStrategy, Component, inject, OnInit, viewChild } from '@angular/core';
import { FormsModule } from '@angular/forms';
import { LucideAngularModule } from 'lucide-angular';

import { EmbindModule as LightenModule } from '../../assets/web_lighten.js';
import { DialogPopupComponent } from '../templates/dialog-popup.component';
import { TextareaTwoComponent } from '../templates/textarea-two.component';
import { WasmLoaderLightenService } from '../wasm-loader-lighten.service';

@Component({
  selector: 'app-lighten',
  imports: [
    TextareaTwoComponent,
    LucideAngularModule,
    FormsModule,
    DialogPopupComponent,
  ],
  templateUrl: './lighten.component.html',
  styleUrl: './lighten.component.css',
  changeDetection: ChangeDetectionStrategy.OnPush,
})
export class AppLightenComponent implements OnInit {
  private lighten?: LightenModule;

  private readonly textareaTwo = viewChild.required(TextareaTwoComponent);

  private readonly wasmLoaderLighten = inject(WasmLoaderLightenService);

  protected count = 4;

  constructor() {
    this.lightenNumber = this.lightenNumber.bind(this);
  }

  async ngOnInit() {
    const count = localStorage.getItem('lighten-count');
    if (count) {
      this.count = Number(count);
    }

    await this.loadWasmLightenModule();
  }

  private async loadWasmLightenModule() {
    this.lighten ??= await this.wasmLoaderLighten.wasm();
  }

  private roundNumbers(data: any): any {
    if (typeof data === 'number') {
      return Number(
        this.lighten!.web_lighten_number(data.toString(), this.count)
      );
    } else if (Array.isArray(data)) {
      return data.map(this.roundNumbers);
    } else if (typeof data === 'object' && data !== null) {
      const roundedObject: any = {};
      for (const key in data) {
        if (data.hasOwnProperty(key)) {
          roundedObject[key] = this.roundNumbers(data[key]);
        }
      }
      return roundedObject;
    }
    return data;
  }

  private processJson(input: string): string {
    try {
      const parsedJson = JSON.parse(input);
      const roundedJson = this.roundNumbers(parsedJson);
      return JSON.stringify(roundedJson, null, 2);
    } catch (_error) {
      return 'Invalid JSON input';
    }
  }

  protected lightenNumber(input: string): Promise<string> {
    return Promise.resolve(this.processJson(input));
  }

  protected onCount(count: number) {
    this.count = count;

    localStorage.setItem('lighten-count', count.toString());

    const event = new Event('input', { bubbles: true });
    this.textareaTwo().inputElement().nativeElement.dispatchEvent(event);
  }
}
