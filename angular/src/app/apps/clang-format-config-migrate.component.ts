import {
  Component,
  OnInit,
  HostListener,
  ViewChild,
  ChangeDetectionStrategy,
  viewChild,
  signal,
  computed,
} from '@angular/core';
import { FormsModule } from '@angular/forms';

import { TextareaTwoComponent } from '../templates/textarea-two.component';
import { SpinnerLoadingComponent } from '../templates/spinner-loading.component';
import { WasmLoaderClangFormatConfigMigrateService } from '../wasm-loader-clang-format-config-migrate.service';
import {
  EmbindModule as ClangFormatConfigMigrateModule,
  VersionList,
  Version,
} from '../../assets/web_clang_format_config_migrate.js';
import { DialogPopupComponent } from '../templates/dialog-popup.component';
import { LucideAngularModule } from 'lucide-angular';

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
})
export class ClangFormatConfigMigrateComponent implements OnInit {
  clangFormatConfigMigrate?: ClangFormatConfigMigrateModule;
  protected readonly spinnerSize = signal(0);

  titleLoading = '';

  compatibleVersions: SelectItem[] = [];

  compatibleStyles?: string[] = [];

  @ViewChild(TextareaTwoComponent) textareaTwo!: TextareaTwoComponent;
  oldVersion = '';
  newVersion = '';
  defaultStyle = '';
  exportOnlyChangedValue = true;

  constructor(
    private wasmLoaderClangFormatConfigMigrate: WasmLoaderClangFormatConfigMigrateService
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

  async loadWasmLoaderClangFormatConfigMigrate() {
    if (!this.clangFormatConfigMigrate) {
      this.clangFormatConfigMigrate =
        await this.wasmLoaderClangFormatConfigMigrate.wasm();
    }
  }

  protected readonly isLoading = computed(()=> {
    if (this.wasmLoaderClangFormatConfigMigrate.isLoading()) {
      this.titleLoading = 'clang-format config migrate';
      return true;
    }
    this.titleLoading = '';
    return false;
  });

  updateIconSize() {
    this.spinnerSize.set(Math.min(window.innerWidth / 4, window.innerHeight / 2));
  }

  async updateCompatibleVersions() {
    await this.loadWasmLoaderClangFormatConfigMigrate();

    const versions: VersionList =
      this.clangFormatConfigMigrate!.getCompatibleVersion(
        this.textareaTwo.inputElement.nativeElement!.value
      );
    this.compatibleVersions = [];

    for (let index = 0; index < versions.size(); index++) {
      const version = versions.get(index)!;
      const element: SelectItem = {
        id: version,
        name: this.clangFormatConfigMigrate!.versionEnumToString(version),
      };
      this.compatibleVersions.push(element);
    }
  }

  async updateCompatibleStyles() {
    await this.loadWasmLoaderClangFormatConfigMigrate();

    this.compatibleStyles = [];

    let oldVersion = this.getOldVersion();
    if (oldVersion === undefined) {
      return;
    }

    if (this.compatibleVersions.length != 0) {
      const compatibleStylesCpp =
        this.clangFormatConfigMigrate!.getStyleNamesRange(
          oldVersion,
          this.clangFormatConfigMigrate!.versionStringToEnum(this.newVersion)
        );
      const sizeStyles = compatibleStylesCpp.size();
      for (let i = 0; i < sizeStyles; i++) {
        this.compatibleStyles.push(compatibleStylesCpp.get(i)!.toString());
      }
    }
  }

  getOldVersion(): Version | undefined {
    let retval: Version;

    if (this.compatibleVersions.length == 0) {
      return undefined;
    }

    if (this.oldVersion == 'min') {
      retval = this.compatibleVersions[0].id;
    } else if (this.oldVersion == 'max') {
      retval = this.compatibleVersions[this.compatibleVersions.length - 1].id;
    } else {
      retval = this.clangFormatConfigMigrate!.versionStringToEnum(
        this.oldVersion
      );
    }

    return retval;
  }

  async migrate(config: string): Promise<string> {
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

  forceMigrate() {
    const event = new Event('input', { bubbles: true });
    this.textareaTwo.inputElement.nativeElement.dispatchEvent(event);
  }
}
