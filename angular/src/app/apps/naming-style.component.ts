import { CommonModule } from '@angular/common';
import { Component, OnInit } from '@angular/core';
import { LucideAngularModule } from 'lucide-angular';

@Component({
  selector: 'app-naming-style',
  imports: [CommonModule, LucideAngularModule],
  templateUrl: './naming-style.component.html',
  styleUrl: './naming-style.component.css',
  preserveWhitespaces: true
})
export class AppNamingStyleComponent implements OnInit {
  checkedSet = new Set<string>();

  async ngOnInit() {
    const naming_style = localStorage.getItem("naming_style");
    if (naming_style) {
      this.checkedSet = new Set(JSON.parse(naming_style));
    }
  }

  private saveToLocalStorage() {
    console.log(JSON.stringify(Array.from(this.checkedSet)));
    localStorage.setItem("naming_style", JSON.stringify(Array.from(this.checkedSet)));
  }

  onCheckboxChange(event: Event) {
    const checkbox = event.target as HTMLInputElement;
    if (checkbox.checked) {
      this.checkedSet.add(checkbox.id.split('_')[0]);
    } else {
      this.checkedSet.delete(checkbox.id.split('_')[0]);
    }
    this.saveToLocalStorage();
  }

  isChecked(id: string): boolean {
    return this.checkedSet.has(id);
  }

  reset() {
    this.checkedSet.clear();
    this.saveToLocalStorage();
  }

  saveToYaml() {
    navigator.clipboard.writeText(
      `CheckOptions:\n${ 
      Array.from(this.checkedSet)
        .map((key) => `  - key: readability-identifier-naming.${key}Case\n    value: 'aNy_CasE'`)
        .join("\n")}`
    );
  }

  setMinimum() {
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
