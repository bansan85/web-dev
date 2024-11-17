import { TestBed } from '@angular/core/testing';
import { AppComponent } from './app.component';
import { importProvidersFrom } from '@angular/core';
import { LucideAngularModule, Settings, LoaderCircle, X } from 'lucide-angular';

describe('AppComponent', () => {
  beforeEach(async () => {
    await TestBed.configureTestingModule({
      imports: [AppComponent],
      providers: [
        importProvidersFrom(
          LucideAngularModule.pick({ Settings, LoaderCircle, X })
        ),
      ],
    }).compileComponents();
  });

  it('should create the app', () => {
    const fixture = TestBed.createComponent(AppComponent);
    const app = fixture.componentInstance;
    expect(app).toBeTruthy();
  });

  it('configuration of clang-format', async () => {
    const fixture = TestBed.createComponent(AppComponent);
    const app = fixture.componentInstance;

    // Test dialog is not opened
    expect(app.isOpen).toBeFalse();

    // Test the loading of wasm for formatStyle
    expect(app.formatStyle).toBeUndefined();

    await app.loadWasmDemanglerModule();
    await app.loadWasmFormatterModule();

    fixture.detectChanges();

    // Test open clang-format dialog
    const settingsImage = <HTMLElement>(
      document.querySelector('lucide-icon[name="settings"]')
    );

    expect(settingsImage).toBeDefined();

    settingsImage!.click();

    await fixture.whenStable();

    const checkedClangFormat = <HTMLElement>(
      document.querySelector('input[name="enableClangFormat"]')
    );
    checkedClangFormat.click();
    fixture.detectChanges();
    await fixture.whenStable();

    expect(app.formatStyle).toBeDefined();

    // Test formatStyle.XXX number
    {
      const dtAccessModifierOffset = Array.from(
        document.querySelectorAll('dt')
      ).find((dt) => dt.textContent! === 'AccessModifierOffset:')!;
      const iAccessModifierOffset =
        dtAccessModifierOffset.nextElementSibling!.querySelector('input')!;
      expect(iAccessModifierOffset.getAttribute('type')).toEqual('number');
      expect(iAccessModifierOffset.valueAsNumber).toEqual(
        app.formatStyle!.AccessModifierOffset
      );
      iAccessModifierOffset.value = '-1';
      iAccessModifierOffset.dispatchEvent(new Event('input'));
      fixture.detectChanges();
      expect(app.formatStyle!.AccessModifierOffset).toEqual(-1);
    }

    // Test formatStyle.XXX boolean
    {
      const dtBinPackParameters = Array.from(
        document.querySelectorAll('dt')
      ).find((dt) => dt.textContent! === 'BinPackParameters:')!;
      const iBinPackParameters =
        dtBinPackParameters.nextElementSibling!.querySelector('input')!;
      expect(iBinPackParameters.getAttribute('type')).toEqual('checkbox');
      const iBinPackParametersValue = iBinPackParameters.checked;
      expect(iBinPackParametersValue).toEqual(
        app.formatStyle!.BinPackParameters
      );
      iBinPackParameters.click();
      fixture.detectChanges();
      expect(app.formatStyle!.BinPackParameters).toEqual(
        !iBinPackParametersValue
      );
    }

    // Test formatStyle.XXX string
    {
      const dtMacroBlockBegin = Array.from(
        document.querySelectorAll('dt')
      ).find((dt) => dt.textContent! === 'MacroBlockBegin:')!;
      const iMacroBlockBegin =
        dtMacroBlockBegin.nextElementSibling!.querySelector('input')!;
      expect(iMacroBlockBegin.getAttribute('type')).toEqual('text');
      expect(iMacroBlockBegin.value).toEqual(app.formatStyle!.MacroBlockBegin);
      iMacroBlockBegin.value = 'Hello';
      iMacroBlockBegin.dispatchEvent(new Event('input'));
      fixture.detectChanges();
      expect(app.formatStyle!.MacroBlockBegin).toEqual('Hello');
    }

    // Test formatStyle.XXX enum
    {
      const dtAlignAfterOpenBracket = Array.from(
        document.querySelectorAll('dt')
      ).find((dt) => dt.textContent! === 'AlignAfterOpenBracket:')!;
      const iAlignAfterOpenBracket =
        dtAlignAfterOpenBracket.nextElementSibling!.querySelector('select')!;
      expect(iAlignAfterOpenBracket.length).toEqual(4);
      expect(iAlignAfterOpenBracket.options[0].text).toEqual('Align');
      const iAlignAfterOpenBracketValue = iAlignAfterOpenBracket.selectedIndex;
      expect(iAlignAfterOpenBracketValue).toEqual(
        app.formatStyle!.AlignAfterOpenBracket.value
      );
      iAlignAfterOpenBracket.value =
        iAlignAfterOpenBracket.options[
          (iAlignAfterOpenBracketValue + 1) % iAlignAfterOpenBracket.length
        ].value;
      iAlignAfterOpenBracket.dispatchEvent(new Event('change'));
      fixture.detectChanges();
      expect(app.formatStyle!.AlignAfterOpenBracket.value).toEqual(
        ((iAlignAfterOpenBracketValue + 1) %
          iAlignAfterOpenBracket.length) as any
      );
    }

    // Test formatStyle.XXX optional
    {
      const dtBracedInitializerIndentWidth = Array.from(
        document.querySelectorAll('dt')
      ).find((dt) => dt.textContent! === 'BracedInitializerIndentWidth:')!;
      const cBracedInitializerIndentWidth =
        dtBracedInitializerIndentWidth.nextElementSibling!.querySelector(
          'input'
        )!;
      expect(cBracedInitializerIndentWidth.getAttribute('type')).toEqual(
        'checkbox'
      );
      const iBracedInitializerIndentWidth = <HTMLInputElement>(
        cBracedInitializerIndentWidth.nextElementSibling
      );
      expect(iBracedInitializerIndentWidth.getAttribute('type')).toEqual(
        'number'
      );
      const cBracedInitializerIndentWidthValue =
        cBracedInitializerIndentWidth.checked;
      expect(cBracedInitializerIndentWidthValue).toEqual(
        typeof app.formatStyle!.BracedInitializerIndentWidth !== 'undefined'
      );
      expect(cBracedInitializerIndentWidthValue).toEqual(
        !iBracedInitializerIndentWidth.disabled
      );
      cBracedInitializerIndentWidth.click();
      fixture.detectChanges();
      expect(cBracedInitializerIndentWidth.checked).toEqual(
        !cBracedInitializerIndentWidthValue
      );
      expect(!iBracedInitializerIndentWidth.disabled).toEqual(
        !cBracedInitializerIndentWidthValue
      );
      expect(
        typeof app.formatStyle!.BracedInitializerIndentWidth !== 'undefined'
      ).toEqual(!cBracedInitializerIndentWidthValue);
      iBracedInitializerIndentWidth.value = '12';
      iBracedInitializerIndentWidth.dispatchEvent(new Event('input'));
      fixture.detectChanges();
      expect(app.formatStyle!.BracedInitializerIndentWidth).toEqual(12);
      cBracedInitializerIndentWidth.click();
      fixture.detectChanges();
      expect(cBracedInitializerIndentWidth.checked).toEqual(
        cBracedInitializerIndentWidthValue
      );
      expect(!iBracedInitializerIndentWidth.disabled).toEqual(
        cBracedInitializerIndentWidthValue
      );
      expect(
        typeof app.formatStyle!.BracedInitializerIndentWidth !== 'undefined'
      ).toEqual(cBracedInitializerIndentWidthValue);
    }

    // Test formatStyle.XXX StringList
    {
      const dtWhitespaceSensitiveMacros = Array.from(
        document.querySelectorAll('dt')
      ).find((dt) => dt.textContent! === 'WhitespaceSensitiveMacros:')!;
      const tWhitespaceSensitiveMacros =
        dtWhitespaceSensitiveMacros.nextElementSibling!.querySelector(
          'textarea'
        )!;
      let stringList: string[] = [];
      for (
        let i = 0;
        i < app.formatStyle!.WhitespaceSensitiveMacros.size();
        i++
      ) {
        stringList.push(
          app.formatStyle!.WhitespaceSensitiveMacros.get(i)!.toString()
        );
      }
      expect(tWhitespaceSensitiveMacros.value).toEqual(stringList.join('\n'));
      tWhitespaceSensitiveMacros.value = 'Hello\nYou';
      tWhitespaceSensitiveMacros.dispatchEvent(new Event('input'));
      fixture.detectChanges();
      expect(app.formatStyle!.WhitespaceSensitiveMacros.size()).toEqual(2);
      let stringList2: string[] = [];
      for (
        let i = 0;
        i < app.formatStyle!.WhitespaceSensitiveMacros.size();
        i++
      ) {
        stringList2.push(
          app.formatStyle!.WhitespaceSensitiveMacros.get(i)!.toString()
        );
      }
      expect(tWhitespaceSensitiveMacros.value).toEqual(stringList2.join('\n'));
    }

    // Test formatStyle.XXX list except StringList
    {
      const dtRawStringFormats = Array.from(
        document.querySelectorAll('dt')
      ).find((dt) => dt.textContent! === 'RawStringFormats:')!;
      const iRawStringFormatsSize =
        dtRawStringFormats.nextElementSibling!.querySelector('input')!;
      expect(iRawStringFormatsSize.getAttribute('type')).toEqual('number');
      expect(Number(iRawStringFormatsSize.value)).toEqual(
        app.formatStyle!.RawStringFormats.size()
      );
      iRawStringFormatsSize.value = '3';
      iRawStringFormatsSize.dispatchEvent(new Event('input'));
      fixture.detectChanges();
      expect(app.formatStyle!.RawStringFormats.size()).toEqual(3);

      expect(
        Array.from(
          dtRawStringFormats.nextElementSibling!.querySelectorAll('dt')
        ).filter((x) => x.textContent!.match(/Item \d+/)).length
      ).toEqual(3);

      Array.from(dtRawStringFormats.nextElementSibling!.querySelectorAll('dt'))
        .filter((x) => x.textContent! === 'Language:')
        .forEach((x, i) => {
          const nextInput = x.nextElementSibling!.querySelector('select')!;
          nextInput.selectedIndex = i+1;
          nextInput.dispatchEvent(new Event('change'));
          fixture.detectChanges();
        });

      expect(app.formatStyle!.RawStringFormats.get(0)!.Language.value).toEqual(
        1
      );
      expect(app.formatStyle!.RawStringFormats.get(1)!.Language.value).toEqual(
        2
      );
      expect(app.formatStyle!.RawStringFormats.get(2)!.Language.value).toEqual(
        3
      );

      // Check that reducing the size of the list don't lose existing data.
      iRawStringFormatsSize.value = '2';
      iRawStringFormatsSize.dispatchEvent(new Event('input'));
      fixture.detectChanges();
      expect(app.formatStyle!.RawStringFormats.size()).toEqual(2);
      expect(app.formatStyle!.RawStringFormats.get(0)?.Language.value).toEqual(
        1
      );
      expect(app.formatStyle!.RawStringFormats.get(1)?.Language.value).toEqual(
        2
      );

      // Check formatStyleKeys
      const allItems = Array.from(
        dtRawStringFormats.nextElementSibling!.querySelectorAll('dt')
      ).filter((x) => x.textContent!.match(/Item \d+/));
      expect(allItems.length).toEqual(2);
      const allItemsI =
        allItems[0].nextElementSibling!.querySelectorAll('dl dt');
      expect(allItemsI.length).toEqual(5);
      const allItemsIExpected: string[] = [
        'Language:',
        'Delimiters:',
        'EnclosingFunctions:',
        'CanonicalDelimiter:',
        'BasedOnStyle:',
      ];
      allItemsI.forEach((x, i) =>
        expect(x.textContent).toEqual(allItemsIExpected[i])
      );
    }

    // Can't test formatStyle.XXX[i].YYY number
    // Can't test formatStyle.XXX[i].YYY boolean

    // Test formatStyle.XXX[i].YYY string
    {
      const dtCanonicalDelimiter = Array.from(
        Array.from(document.querySelectorAll('dt'))
          .find((dt) => dt.textContent! === 'RawStringFormats:')!
          .nextElementSibling!.querySelectorAll('dl dd dl dt')
      ).find((dt) => dt.textContent! === 'CanonicalDelimiter:')!;
      const iCanonicalDelimiter =
        dtCanonicalDelimiter.nextElementSibling!.querySelector('input')!;
      expect(iCanonicalDelimiter.getAttribute('type')).toEqual('text');
      expect(iCanonicalDelimiter.value).toEqual(
        app.formatStyle!.RawStringFormats.get(0)!.CanonicalDelimiter
      );
      iCanonicalDelimiter.value = 'Hello';
      iCanonicalDelimiter.dispatchEvent(new Event('input'));
      fixture.detectChanges();
      expect(
        app.formatStyle!.RawStringFormats.get(0)!.CanonicalDelimiter
      ).toEqual('Hello');
    }

    // Test formatStyle.XXX[i].YYY StringList
    {
      const dtLanguage = Array.from(
        Array.from(document.querySelectorAll('dt'))
          .find((dt) => dt.textContent! === 'RawStringFormats:')!
          .nextElementSibling!.querySelectorAll('dl dd dl dt')
      ).find((dt) => dt.textContent! === 'Language:')!;
      const iLanguage = dtLanguage.nextElementSibling!.querySelector('select')!;
      expect(iLanguage.length).toEqual(11);
      expect(iLanguage.options[0].text).toEqual('None');
      const iLanguageValue = iLanguage.selectedIndex;
      expect(iLanguage.selectedIndex).toEqual(1);
      expect(iLanguageValue).toEqual(
        app.formatStyle!.RawStringFormats.get(0)!.Language.value
      );
      iLanguage.value =
        iLanguage.options[(iLanguageValue + 5) % iLanguage.length].value;
      iLanguage.dispatchEvent(new Event('change'));
      fixture.detectChanges();
      expect(app.formatStyle!.RawStringFormats.get(0)!.Language.value).toEqual(
        ((iLanguageValue + 5) % iLanguage.length) as any
      );
    }

    // Can't test formatStyle.XXX[i].YYY optional

    // Test formatStyle.XXX[i].YYY StringList
    {
      const dtDelimiters = Array.from(
        Array.from(document.querySelectorAll('dt'))
          .find((dt) => dt.textContent! === 'RawStringFormats:')!
          .nextElementSibling!.querySelectorAll('dl dd dl dt')
      ).find((dt) => dt.textContent! === 'Delimiters:')!;
      const tDelimiters =
        dtDelimiters.nextElementSibling!.querySelector('textarea')!;
      let stringList: string[] = [];
      for (
        let i = 0;
        i < app.formatStyle!.RawStringFormats.get(0)!.Delimiters.size();
        i++
      ) {
        stringList.push(
          app
            .formatStyle!.RawStringFormats.get(0)!
            .Delimiters.get(i)!
            .toString()
        );
      }
      expect(tDelimiters.value).toEqual(stringList.join('\n'));
      tDelimiters.value = 'Hello\nYou\nToo';
      tDelimiters.dispatchEvent(new Event('input'));
      fixture.detectChanges();
      expect(
        app.formatStyle!.RawStringFormats.get(0)!.Delimiters.size()
      ).toEqual(3);
      let stringList2: string[] = [];
      for (
        let i = 0;
        i < app.formatStyle!.RawStringFormats.get(0)!.Delimiters.size();
        i++
      ) {
        stringList2.push(
          app
            .formatStyle!.RawStringFormats.get(0)!
            .Delimiters.get(i)!
            .toString()
        );
      }
      expect(tDelimiters.value).toEqual(stringList2.join('\n'));
    }

    // Test formatStyle.XXX.YYY number
    {
      const dtOverEmptyLines = Array.from(document.querySelectorAll('dt')).find(
        (dt) => dt.textContent! === 'OverEmptyLines:'
      )!;
      const iOverEmptyLines =
        dtOverEmptyLines.nextElementSibling!.querySelector('input')!;
      expect(iOverEmptyLines.getAttribute('type')).toEqual('number');
      expect(iOverEmptyLines.valueAsNumber).toEqual(
        app.formatStyle!.AlignTrailingComments.OverEmptyLines
      );
      iOverEmptyLines.value = '2';
      iOverEmptyLines.dispatchEvent(new Event('input'));
      fixture.detectChanges();
      expect(app.formatStyle!.AlignTrailingComments.OverEmptyLines).toEqual(2);
    }

    // Test formatStyle.XXX.YYY boolean
    {
      const dtAtStartOfBlock = Array.from(document.querySelectorAll('dt')).find(
        (dt) => dt.textContent! === 'AtStartOfBlock:'
      )!;
      const iAtStartOfBlock =
        dtAtStartOfBlock.nextElementSibling!.querySelector('input')!;
      expect(iAtStartOfBlock.getAttribute('type')).toEqual('checkbox');
      const iAtStartOfBlockValue = iAtStartOfBlock.checked;
      expect(iAtStartOfBlockValue).toEqual(
        app.formatStyle!.KeepEmptyLines.AtStartOfBlock
      );
      iAtStartOfBlock.click();
      fixture.detectChanges();
      expect(app.formatStyle!.KeepEmptyLines.AtStartOfBlock).toEqual(
        !iAtStartOfBlockValue
      );
    }

    // Test formatStyle.XXX.YYY string
    {
      const dtIncludeIsMainRegex = Array.from(
        document.querySelectorAll('dt')
      ).find((dt) => dt.textContent! === 'IncludeIsMainRegex:')!;
      const iIncludeIsMainRegex =
        dtIncludeIsMainRegex.nextElementSibling!.querySelector('input')!;
      expect(iIncludeIsMainRegex.getAttribute('type')).toEqual('text');
      expect(iIncludeIsMainRegex.value).toEqual(
        app.formatStyle!.IncludeStyle.IncludeIsMainRegex
      );
      iIncludeIsMainRegex.value = 'Hello';
      iIncludeIsMainRegex.dispatchEvent(new Event('input'));
      fixture.detectChanges();
      expect(app.formatStyle!.IncludeStyle.IncludeIsMainRegex).toEqual('Hello');
    }

    // Test formatStyle.XXX.YYY enum
    {
      const dtKind = Array.from(document.querySelectorAll('dt')).find(
        (dt) => dt.textContent! === 'Kind:'
      )!;
      const iKind = dtKind.nextElementSibling!.querySelector('select')!;
      expect(iKind.length).toEqual(3);
      expect(iKind.options[0].text).toEqual('Leave');
      const iKindValue = iKind.selectedIndex;
      expect(iKindValue).toEqual(
        app.formatStyle!.AlignTrailingComments.Kind.value
      );
      iKind.value = iKind.options[(iKindValue + 1) % iKind.length].value;
      iKind.dispatchEvent(new Event('change'));
      fixture.detectChanges();
      expect(app.formatStyle!.AlignTrailingComments.Kind.value).toEqual(
        ((iKindValue + 1) % iKind.length) as any
      );
    }

    // Can't test formatStyle.XXX.YYY optional
    // Can't test formatStyle.XXX.YYY StringList

    // 115
    // Test formatStyle.XXX.YYY list except StringList
    {
      const dtIncludeCategories = Array.from(
        document.querySelectorAll('dt')
      ).find((dt) => dt.textContent! === 'IncludeCategories:')!;
      const iIncludeCategoriesSize =
        dtIncludeCategories.nextElementSibling!.querySelector('input')!;
      expect(iIncludeCategoriesSize.getAttribute('type')).toEqual('number');
      expect(Number(iIncludeCategoriesSize.value)).toEqual(
        app.formatStyle!.IncludeStyle.IncludeCategories.size()
      );
      iIncludeCategoriesSize.value = '3';
      iIncludeCategoriesSize.dispatchEvent(new Event('input'));
      fixture.detectChanges();
      expect(app.formatStyle!.IncludeStyle.IncludeCategories.size()).toEqual(3);

      expect(
        Array.from(
          dtIncludeCategories.nextElementSibling!.querySelectorAll('dt')
        ).filter((x) => x.textContent!.match(/Item \d+/)).length
      ).toEqual(3);

      Array.from(dtIncludeCategories.nextElementSibling!.querySelectorAll('dt'))
        .filter((x) => x.textContent! === 'Priority:')
        .forEach((x, i) => {
          const nextInput = x.nextElementSibling!.querySelector('input')!;
          nextInput.value = i.toString();
          nextInput.dispatchEvent(new Event('input'));
          fixture.detectChanges();
        });

      expect(
        app.formatStyle!.IncludeStyle.IncludeCategories.get(0)!.Priority
      ).toEqual(0);
      expect(
        app.formatStyle!.IncludeStyle.IncludeCategories.get(1)!.Priority
      ).toEqual(1);
      expect(
        app.formatStyle!.IncludeStyle.IncludeCategories.get(2)!.Priority
      ).toEqual(2);

      // Check that reducing the size of the list don't lose existing data.
      iIncludeCategoriesSize.value = '2';
      iIncludeCategoriesSize.dispatchEvent(new Event('input'));
      fixture.detectChanges();
      expect(app.formatStyle!.IncludeStyle.IncludeCategories.size()).toEqual(2);
      expect(
        app.formatStyle!.IncludeStyle.IncludeCategories.get(0)?.Priority
      ).toEqual(0);
      expect(
        app.formatStyle!.IncludeStyle.IncludeCategories.get(1)?.Priority
      ).toEqual(1);

      // Check formatStyleKeys
      const allItems = Array.from(
        dtIncludeCategories.nextElementSibling!.querySelectorAll('dt')
      ).filter((x) => x.textContent!.match(/Item \d+/));
      expect(allItems.length).toEqual(2);
      const allItemsI =
        allItems[0].nextElementSibling!.querySelectorAll('dl dt');
      expect(allItemsI.length).toEqual(4);
      const allItemsIExpected: string[] = [
        'Regex:',
        'Priority:',
        'SortPriority:',
        'RegexIsCaseSensitive:',
      ];
      allItemsI.forEach((x, i) =>
        expect(x.textContent).toEqual(allItemsIExpected[i])
      );
    }

    // Test formatStyle.XXX.YYY[i].ZZZ number
    {
      const dtSortPriority = Array.from(document.querySelectorAll('dt')).find(
        (dt) => dt.textContent! === 'SortPriority:'
      )!;
      const iSortPriority =
        dtSortPriority.nextElementSibling!.querySelector('input')!;
      expect(iSortPriority.getAttribute('type')).toEqual('number');
      expect(iSortPriority.valueAsNumber).toEqual(
        app.formatStyle!.IncludeStyle.IncludeCategories.get(0)!.SortPriority
      );
      iSortPriority.value = '-3';
      iSortPriority.dispatchEvent(new Event('input'));
      fixture.detectChanges();
      expect(
        app.formatStyle!.IncludeStyle.IncludeCategories.get(0)!.SortPriority
      ).toEqual(-3);
    }

    // Test formatStyle.XXX.YYY[i].ZZZ boolean
    {
      const dtRegexIsCaseSensitive = Array.from(
        document.querySelectorAll('dt')
      ).find((dt) => dt.textContent! === 'RegexIsCaseSensitive:')!;
      const iRegexIsCaseSensitive =
        dtRegexIsCaseSensitive.nextElementSibling!.querySelector('input')!;
      expect(iRegexIsCaseSensitive.getAttribute('type')).toEqual('checkbox');
      const iRegexIsCaseSensitiveValue = iRegexIsCaseSensitive.checked;
      expect(iRegexIsCaseSensitiveValue).toEqual(
        app.formatStyle!.IncludeStyle.IncludeCategories.get(0)!
          .RegexIsCaseSensitive
      );
      iRegexIsCaseSensitive.click();
      fixture.detectChanges();
      expect(
        app.formatStyle!.IncludeStyle.IncludeCategories.get(0)!
          .RegexIsCaseSensitive
      ).toEqual(!iRegexIsCaseSensitiveValue);
    }

    // Test formatStyle.XXX.YYY[i].ZZZ string
    {
      const dtRegex = Array.from(document.querySelectorAll('dt')).find(
        (dt) => dt.textContent! === 'Regex:'
      )!;
      const iRegex = dtRegex.nextElementSibling!.querySelector('input')!;
      expect(iRegex.getAttribute('type')).toEqual('text');
      expect(iRegex.value).toEqual(
        app.formatStyle!.IncludeStyle.IncludeCategories.get(0)!.Regex
      );
      iRegex.value = 'Hello';
      iRegex.dispatchEvent(new Event('input'));
      fixture.detectChanges();
      expect(
        app.formatStyle!.IncludeStyle.IncludeCategories.get(0)!.Regex
      ).toEqual('Hello');
    }

    // Can't test formatStyle.XXX.YYY[i].ZZZ enum
    // Can't test formatStyle.XXX.YYY[i].ZZZ optional
    // Can't test formatStyle.XXX.YYY[i].ZZZ StringList

    {
      const spans = document.querySelectorAll('span');
      const notImplementedSpans = Array.from(spans).filter((span) =>
        span.textContent?.startsWith('Not implemented')
      );
      expect(notImplementedSpans.length).toEqual(0);
    }
  });
});
