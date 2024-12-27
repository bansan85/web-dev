import { Component } from '@angular/core';

import { TextareaTwoComponent } from '../templates/textarea-two.component';

@Component({
  selector: 'app-lighten',
  imports: [TextareaTwoComponent],
  templateUrl: './lighten.component.html',
  styleUrl: './lighten.component.css',
})
export class AppLightenComponent {
  constructor() {
    this.lighten = this.lighten.bind(this);
  }

  roundNumbers(data: any): any {
    if (typeof data === 'number') {
      return Math.round(data);
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

  processJson(input: string): string {
    try {
      const parsedJson = JSON.parse(input);
      const roundedJson = this.roundNumbers(parsedJson);
      return JSON.stringify(roundedJson, null, 2);
    } catch (error) {
      return 'Invalid JSON input';
    }
  }

  async lighten(input: string): Promise<string> {
    return this.processJson(input);
  }
}
