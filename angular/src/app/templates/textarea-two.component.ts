import { CommonModule } from '@angular/common';
import { Component, ElementRef, Input, ViewChild } from '@angular/core';

@Component({
  selector: 'app-textarea-two',
  imports: [CommonModule],
  templateUrl: './textarea-two.component.html',
  styleUrl: './textarea-two.component.css'
})
export class TextareaTwoComponent {
  @ViewChild('inputElement') inputElement!: ElementRef<HTMLTextAreaElement>;

  @Input() inputPlaceholder!: string;

  @Input() inputChange!: (input: string) => Promise<string>;

  outputStr: Promise<string> = Promise.resolve('');

  inputToOutput(input: string) {
    this.outputStr = this.inputChange(input);
  }
}
