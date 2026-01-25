import { CommonModule } from '@angular/common';
import { ChangeDetectionStrategy, Component, ElementRef, input, viewChild } from '@angular/core';

@Component({
  selector: 'app-textarea-two',
  imports: [CommonModule],
  templateUrl: './textarea-two.component.html',
  styleUrl: './textarea-two.component.css',
  changeDetection: ChangeDetectionStrategy.OnPush,
})
export class TextareaTwoComponent {
  readonly inputElement = viewChild.required<ElementRef<HTMLTextAreaElement>>('inputElement');

  readonly inputPlaceholder = input.required<string>();

  readonly inputChange = input.required<(input: string) => Promise<string>>();

  protected outputStr: Promise<string> = Promise.resolve('');

  inputToOutput(inputStr: string) {
    this.outputStr = this.inputChange()(inputStr);
  }
}
