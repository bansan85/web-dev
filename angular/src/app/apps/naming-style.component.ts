import { CommonModule } from '@angular/common';
import { ChangeDetectionStrategy, Component, ElementRef, OnInit, viewChild } from '@angular/core';
import { LucideAngularModule } from 'lucide-angular';

import { GithubMarkInlineComponent } from '../img/github-mark-inline.component';
import { DialogPopupComponent } from '../templates/dialog-popup.component';

@Component({
  selector: 'app-naming-style',
  imports: [DialogPopupComponent, CommonModule, LucideAngularModule, GithubMarkInlineComponent],
  templateUrl: './naming-style.component.html',
  styleUrl: './naming-style.component.css',
  changeDetection: ChangeDetectionStrategy.OnPush,
  preserveWhitespaces: true,
})
export class AppNamingStyleComponent implements OnInit {
  private readonly dialog = viewChild.required<DialogPopupComponent>('dialog');
  private readonly namingText = viewChild.required<ElementRef<HTMLTextAreaElement>>('namingText');

  private checkedSet = new Set<string>();

  ngOnInit() {
    const namingStyle = localStorage.getItem("naming_style");
    if (namingStyle) {
      this.checkedSet = new Set(JSON.parse(namingStyle));
    }
  }

  private saveToLocalStorage() {
    localStorage.setItem("naming_style", JSON.stringify(Array.from(this.checkedSet)));
  }

  protected onCheckboxChange(event: Event) {
    const checkbox = event.target as HTMLInputElement;
    if (checkbox.checked) {
      this.checkedSet.add(checkbox.id.split('_')[0]);
    } else {
      this.checkedSet.delete(checkbox.id.split('_')[0]);
    }
    this.saveToLocalStorage();
  }

  protected isChecked(id: string): boolean {
    return this.checkedSet.has(id);
  }

  protected reset() {
    this.checkedSet.clear();
    this.saveToLocalStorage();
  }

  protected showImport() {
    this.dialog().openDialog();
  }

  protected importFromText() {
    this.checkedSet.clear();
    this.namingText().nativeElement.value.split('\n')
      .filter(line => line.includes('readability-identifier-naming.'))
      .map(line => {
        const match = /readability-identifier-naming\.(?<name>.+?)Case/u.exec(line);
        return match?.groups?.['name'] ?? '';
      })
      .filter(Boolean)
      .forEach(item => this.checkedSet.add(item));
    this.dialog().closeDialog();
  }

  protected async saveToYaml() {
    await navigator.clipboard.writeText(
      `CheckOptions:\n${Array.from(this.checkedSet)
        .map((key) => `  - key: readability-identifier-naming.${key}Case\n    value: 'aNy_CasE'`)
        .join("\n")}`
    );
  }

  protected setMinimum() {
    this.checkedSet.clear();
    this.checkedSet.add('Typedef');
    this.checkedSet.add('TypeAlias');
    this.checkedSet.add('Namespace');
    this.checkedSet.add('Enum');
    this.checkedSet.add('EnumConstant');
    this.checkedSet.add('Class');
    this.checkedSet.add('Union');
    this.checkedSet.add('Member');
    this.checkedSet.add('Parameter');
    this.checkedSet.add('LocalVariable');
    this.checkedSet.add('GlobalVariable');
    this.checkedSet.add('ClassMember');
    this.checkedSet.add('Function');
    this.checkedSet.add('TemplateParameter');
    this.checkedSet.add('Concept');
    this.checkedSet.add('MacroDefinition');
    this.saveToLocalStorage();
  }
}
