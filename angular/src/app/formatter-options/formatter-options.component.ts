import {
  ChangeDetectionStrategy,
  Component,
  Input,
  output,
} from '@angular/core';
import { FormsModule } from '@angular/forms';

import {
  FormatterModule,
  FormatStyle,
  StringList,
} from '../wasm-loader-formatter.service';

@Component({
  selector: 'app-formatter-options',
  imports: [FormsModule],
  templateUrl: './formatter-options.component.html',
  styleUrl: './formatter-options.component.css',
  changeDetection: ChangeDetectionStrategy.OnPush
})
export class FormatterOptionsComponent {
  @Input({ required: true }) formatter!: FormatterModule;
  @Input({ required: true }) formatStyle!: FormatStyle;
  @Input({ required: true }) emptyStyle!: FormatStyle;

  readonly changeOptions = output();

  private getLastStruct(root: FormatStyle, keys: (string | number)[]) {
    let target = root as any;
    for (let i = 0; i < keys.length - 1; i += 1) {
      if (typeof keys[i] === 'number') {
        target = target.get(keys[i]);
      } else {
        target = target[keys[i]];
      }
    }
    return target;
  }

  protected formatStyleKeys(keys: (string | number)[]): any {
    let target = this.getLastStruct(this.formatStyle, keys);

    if (keys.length !== 0) {
      if (typeof keys.at(-1) === 'number') {
        target = target.get(keys.at(-1));
      } else {
        target = target[keys.at(-1)!];
      }
    }

    const retval: any = [];

    if (!this.formatter) {
      return retval;
    }

    for (const key in target) {
      if (target.hasOwnProperty(key) || key in target) {
        retval.push(key);
      }
    }

    return retval;
  }

  private updateField(keys: (string | number)[], assign: (x: any[]) => void): void {
    const tree: any[] = [];
    let target = this.formatStyle as any;
    for (const key of keys) {
      if (typeof key === 'number') {
        target = target.get(key);
      } else {
        target = target[key];
      }
      tree.push(target);
    }

    assign(tree);

    for (let i = keys.length - 1; i > 0; i -= 1) {
      if (typeof keys[i] === 'number') {
        tree[i - 1].set(keys[i], tree[i]);
        i -= 1;
      } else {
        tree[i - 1][keys[i]] = tree[i];
      }
    }

    (this.formatStyle as any)[keys.at(0)!] = tree[0];

    this.changeOptions.emit();
  }

  protected isNumber(value: unknown): boolean {
    return typeof value === 'number';
  }

  protected minNumber(root_field: any, field: string): number {
    switch (root_field[`get${field}Type`]()) {
      case -8: {
        return -127;
      }
      case 8: {
        return 0;
      }
      case -16: {
        return -32767;
      }
      case 16: {
        return 0;
      }
      case -32: {
        return -2147483647;
      }
      case 32: {
        return 0;
      }
    }
    return 0;
  }

  protected maxNumber(root_field: any, field: string): number {
    switch (root_field[`get${field}Type`]()) {
      case -8: {
        return 0x7f;
      }
      case 8: {
        return 0xff;
      }
      case -16: {
        return 0x7fff;
      }
      case 16: {
        return 0xffff;
      }
      case -32: {
        return 0x7fffffff;
      }
      case 32: {
        return 0xffffffff;
      }
    }
    return 0;
  }

  protected isBoolean(value: unknown): boolean {
    return typeof value === 'boolean';
  }

  protected isString(value: unknown): boolean {
    return typeof value === 'string';
  }

  protected updateRawType(
    newValue: number | boolean | string,
    keys: (string | number)[]
  ): void {
    this.updateField(keys, (x: any[]) => {
      x[x.length - 1] = newValue;
    });
  }

  protected isEnum(value: any): boolean {
    return typeof value === 'object' && typeof value.$$ === 'undefined';
  }

  protected getEnum(value: any): string {
    return value.constructor.name.split('_').slice(2).join('_');
  }

  protected allEnums(value: any): string[] {
    const items = Object.getOwnPropertyNames(
      Object.getPrototypeOf(value).constructor
    );
    return items
      .filter(
        (item) =>
          !['values', 'prototype', 'length', 'name', 'argCount'].includes(item)
      )
      .map((item) => item.split('_').slice(1).join('_'));
  }

  protected updateEnum(newValue: string, keys: (string | number)[]): void {
    this.updateField(keys, (x: any[]) => {
      x[x.length - 1] = (this.formatter as any)[
        x[x.length - 1].constructor.name.split('_')[0]
      ][`${x[x.length - 1].constructor.name.split('_')[1]}_${newValue}`];
    });
  }

  protected isUndefined(value: any): boolean {
    return typeof value === 'undefined';
  }

  protected onUndefinedCheckboxChange(
    event: Event,
    inputValue: string,
    keys: (string | number)[]
  ) {
    this.updateField(keys, (x: any[]) => {
      const { checked } = (event.target as HTMLInputElement);
      x[x.length - 1] = checked
        ? inputValue === ''
          ? 0
          : inputValue
        : undefined;
    });
  }

  protected onUndefinedInputChange(value: string, keys: (string | number)[]) {
    this.updateField(keys, (x: any[]) => {
      if (x[x.length - 1] !== undefined) {
        x[x.length - 1] = Number(value);
      }
    });
  }

  protected isStringList(value: any): boolean {
    return (
      typeof value === 'object' &&
      typeof value.$$ !== 'undefined' &&
      value.$$.ptrType.registeredClass.name === 'StringList'
    );
  }

  protected stringListToTextArea(raw_value: any): string {
    const value: StringList = raw_value as StringList;
    const retval: string[] = [];
    for (let i = 0; i < value.size(); i += 1) {
      retval.push(value.get(i)!);
    }
    return retval.join('\n');
  }

  protected onStringList(event: Event, keys: (string | number)[]): void {
    this.updateField(keys, (x: any[]) => {
      (x[x.length - 1] as StringList).resize(0, '');
      const data: string = (event.target as any).value;
      data
        .split('\n')
        .forEach((dataI) => { (x[x.length - 1] as StringList).push_back(dataI); });
    });
  }

  protected isFunction(value: unknown): boolean {
    return typeof value === 'function';
  }

  protected isList(value: any): boolean {
    return (
      typeof value === 'object' &&
      typeof value.$$ !== 'undefined' &&
      typeof value.push_back === 'function' &&
      typeof value.resize === 'function' &&
      typeof value.get === 'function' &&
      typeof value.set === 'function' &&
      typeof value.size === 'function'
    );
  }

  protected resizeList(value: number, keys: (string | number)[]): void {
    this.updateField(keys, (x: any[]) => {
      if (x[x.length - 1].size() > value) {
        x[x.length - 1].resize(
          value,
          new (this.formatter as any)![
            x[x.length - 1].$$.ptrType.registeredClass.name.slice(0, -4)
          ]()
        );
      } else if (x[x.length - 1].size() < value) {
        for (let i: number = x[x.length - 1].size(); i < value; i += 1) {
          x[x.length - 1].push_back(
            new (this.formatter as any)![
              x[x.length - 1].$$.ptrType.registeredClass.name.slice(0, -4)
            ]()
          );
        }
      }
    });
  }

  protected isMiscStruct(value: any): boolean {
    return (
      typeof value === 'object' &&
      typeof value.$$ !== 'undefined' &&
      !this.isList(value)
    );
  }
}
