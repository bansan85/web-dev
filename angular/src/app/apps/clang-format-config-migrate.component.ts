import {
  ChangeDetectionStrategy,
  Component,
  computed,
  HostListener,
  inject,
  OnInit,
  signal,
  viewChild,
} from '@angular/core';
import { FormsModule } from '@angular/forms';
import { LucideAngularModule } from 'lucide-angular';
import {
  WasmLoaderClangFormatConfigMigrateService,
  ClangFormatConfigMigrateModule,
  VersionList,
  Version,
} from '../wasm-loader-clang-format-config-migrate.service';
import { DialogPopupComponent } from '../templates/dialog-popup.component';
import { SpinnerLoadingComponent } from '../templates/spinner-loading.component';
import { TextareaTwoComponent } from '../templates/textarea-two.component';

interface SelectItem {
  id: Version;
  name: string;
}

@Component({
  selector: 'app-clang-format-config-migrate',
  imports: [
    DialogPopupComponent,
    FormsModule,
    LucideAngularModule,
    TextareaTwoComponent,
    SpinnerLoadingComponent
  ],
  templateUrl: './clang-format-config-migrate.component.html',
  styleUrl: './clang-format-config-migrate.component.css',
  changeDetection: ChangeDetectionStrategy.OnPush,
})
export class ClangFormatConfigMigrateComponent implements OnInit {
  private clangFormatConfigMigrate?: ClangFormatConfigMigrateModule;
  protected readonly spinnerSize = signal(0);

  protected titleLoading = '';

  protected readonly compatibleVersions = signal<SelectItem[]>([]);

  protected readonly compatibleStyles = signal<string[]>([]);

  private readonly textareaTwo = viewChild.required(TextareaTwoComponent);
  private readonly settingsDialog = viewChild.required(DialogPopupComponent);

  protected oldVersion = '';
  protected newVersion = '';
  protected defaultStyle = '';
  protected exportOnlyChangedValue = true;

  private readonly wasmLoaderClangFormatConfigMigrate = inject(WasmLoaderClangFormatConfigMigrateService);

  constructor(
  ) {
    this.migrate = this.migrate.bind(this);
  }

  async ngOnInit() {
    this.updateIconSize();

    await this.loadWasmLoaderClangFormatConfigMigrate();
  }

  @HostListener('window:resize')
  onResize() {
    this.updateIconSize();
  }

  private async loadWasmLoaderClangFormatConfigMigrate() {
    this.clangFormatConfigMigrate ??= await this.wasmLoaderClangFormatConfigMigrate.wasm();
  }

  protected readonly isLoading = computed(() => {
    if (this.wasmLoaderClangFormatConfigMigrate.isLoading()) {
      this.titleLoading = 'clang-format config migrate';
      return true;
    }
    this.titleLoading = '';
    return false;
  });

  private updateIconSize() {
    this.spinnerSize.set(Math.min(window.innerWidth / 4, window.innerHeight / 2));
  }

  private async updateCompatibleVersions() {
    await this.loadWasmLoaderClangFormatConfigMigrate();

    const versions: VersionList =
      this.clangFormatConfigMigrate!.getCompatibleVersion(
        this.textareaTwo().inputElement().nativeElement.value
      );
    const compatibleVersions: SelectItem[] = [];

    for (let index = 0; index < versions.size(); index += 1) {
      const version = versions.get(index)!;
      const element: SelectItem = {
        id: version,
        name: this.clangFormatConfigMigrate!.versionEnumToString(version),
      };
      compatibleVersions.push(element);
    }
    this.compatibleVersions.set(compatibleVersions);
  }

  protected async updateCompatibleStyles() {
    await this.loadWasmLoaderClangFormatConfigMigrate();

    const compatibleStyles: string[] = [];

    const oldVersion = this.getOldVersion();
    if (oldVersion === undefined) {
      return;
    }

    if (this.compatibleVersions().length !== 0) {
      const compatibleStylesCpp =
        this.clangFormatConfigMigrate!.getStyleNamesRange(
          oldVersion,
          this.clangFormatConfigMigrate!.versionStringToEnum(this.newVersion)
        );
      const sizeStyles = compatibleStylesCpp.size();
      for (let i = 0; i < sizeStyles; i += 1) {
        compatibleStyles.push(compatibleStylesCpp.get(i)!);
      }
    }

    this.compatibleStyles.set(compatibleStyles);
  }

  private getOldVersion(): Version | undefined {
    let retval: Version;

    if (this.compatibleVersions().length === 0) {
      return undefined;
    }

    if (this.oldVersion === 'min') {
      retval = this.compatibleVersions()[0].id;
    } else if (this.oldVersion === 'max') {
      retval = this.compatibleVersions()[this.compatibleVersions().length - 1].id;
    } else {
      retval = this.clangFormatConfigMigrate!.versionStringToEnum(
        this.oldVersion
      );
    }

    return retval;
  }

  protected async migrate(config: string): Promise<string> {
    await this.loadWasmLoaderClangFormatConfigMigrate();

    await this.updateCompatibleVersions();

    await this.updateCompatibleStyles();

    const realOldVersion = this.getOldVersion();

    if (realOldVersion === undefined) {
      return '';
    }

    return this.clangFormatConfigMigrate!.migrateTo(
      realOldVersion,
      this.clangFormatConfigMigrate!.versionStringToEnum(this.newVersion),
      config,
      this.defaultStyle,
      this.exportOnlyChangedValue
    );
  }

  protected forceMigrate() {
    const event = new Event('input', { bubbles: true });
    this.textareaTwo().inputElement().nativeElement.dispatchEvent(event);
  }

  protected async openSettings() {
    await this.updateCompatibleVersions();
    this.settingsDialog().openDialog();
  }
}
