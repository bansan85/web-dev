import { Component, OnInit, HostListener, ViewChild, ChangeDetectionStrategy } from '@angular/core';
import { FormsModule } from '@angular/forms';
import { NgIf, NgFor, CommonModule } from '@angular/common';
import { TextareaTwoComponent } from '../templates/textarea-two.component';
import { SpinnerLoadingComponent } from '../templates/spinner-loading.component';
import { WasmLoaderClangFormatConfigMigrateService } from '../wasm-loader-clang-format-config-migrate.service';
import { EmbindModule as ClangFormatConfigMigrateModule, VersionList, Version } from '../../assets/web_clang_format_config_migrate.js';
import { DialogPopupComponent } from '../templates/dialog-popup.component';
import { LucideAngularModule } from 'lucide-angular';

interface SelectItem {
  id: string;
  name: string;
}

@Component({
  selector: 'app-clang-format-config-migrate',
  imports: [
    DialogPopupComponent,
    FormsModule,
    CommonModule,
    NgIf,
    NgFor,
    LucideAngularModule,
    TextareaTwoComponent,
    SpinnerLoadingComponent,
  ],
  templateUrl: './clang-format-config-migrate.component.html',
  styleUrl: './clang-format-config-migrate.component.css',
  changeDetection: ChangeDetectionStrategy.OnPush
})
export class ClangFormatConfigMigrateComponent implements OnInit {
  clangFormatConfigMigrate?: ClangFormatConfigMigrateModule;
  spinnerSize = 0;

  titleLoading = '';

  @ViewChild(TextareaTwoComponent) textareaTwo!: TextareaTwoComponent;

  constructor(
    private wasmLoaderClangFormatConfigMigrate: WasmLoaderClangFormatConfigMigrateService
  ) { }

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

  isLoading(): boolean {
    if (this.wasmLoaderClangFormatConfigMigrate.loading()) {
      this.titleLoading = 'clang-format config migrate';
      return true;
    }
    this.titleLoading = '';
    return false;
  }

  updateIconSize() {
    this.spinnerSize = Math.min(window.innerWidth / 4, window.innerHeight / 2);
  }

  async compatibleVersion(): Promise<SelectItem[]> {
    await this.loadWasmLoaderClangFormatConfigMigrate();

    const versions: VersionList = this.clangFormatConfigMigrate!.getCompatibleVersion(this.textareaTwo.inputElement.nativeElement!.value);
    let retval: SelectItem[] = [];

    for (let index = 0; index < versions.size(); index++) {
      const version = versions.get(index)!;
      const element: SelectItem = { id: JSON.stringify(version), name: JSON.stringify(version) };
      retval.push(element);
    }

    console.log(retval);

    return retval;
  }

  async migrate() {
    return "";
  }
}
