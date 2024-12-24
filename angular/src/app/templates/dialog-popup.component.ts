import {
  Component,
  ViewChild,
  ElementRef,
  Renderer2,
  NgZone,
} from '@angular/core';
import { NgClass } from '@angular/common';
import { LucideAngularModule } from 'lucide-angular';

@Component({
  selector: 'app-dialog-popup',
  imports: [NgClass, LucideAngularModule],
  templateUrl: './dialog-popup.component.html',
  styleUrl: './dialog-popup.component.css'
})
export class DialogPopupComponent {
  @ViewChild('dialog') dialogRef!: ElementRef<HTMLDialogElement>;

  isOpen = false;

  isDragging = false;
  offsetX = 0;
  offsetY = 0;
  mouseMoveListener?: () => void;
  mouseUpListener?: () => void;

  constructor(private renderer: Renderer2, private ngZone: NgZone) { }

  openDialog() {
    this.dialogRef.nativeElement.showModal();
    setTimeout(() => (this.isOpen = true), 0);
    const bounds = this.dialogRef.nativeElement.getBoundingClientRect();
    this.dialogRef.nativeElement.style.margin = '0';
    this.dialogRef.nativeElement.style.left = `${bounds.x}px`;
    this.dialogRef.nativeElement.style.top = `${bounds.y}px`;
  }

  closeDialog() {
    const close = (event: TransitionEvent) => {
      if (event.propertyName === 'opacity') {
        this.dialogRef.nativeElement.close();
        this.dialogRef.nativeElement.removeEventListener(
          'transitionend',
          close
        );
      }
    };

    this.dialogRef.nativeElement.addEventListener('transitionend', close);
    this.isOpen = false;
  }

  onMouseDown(event: MouseEvent): void {
    this.isDragging = true;

    const dialogElement = this.dialogRef.nativeElement;
    this.offsetX = event.clientX - dialogElement.getBoundingClientRect().left;
    this.offsetY = event.clientY - dialogElement.getBoundingClientRect().top;

    this.ngZone.runOutsideAngular(() => {
      this.mouseMoveListener = this.renderer.listen(
        'window',
        'mousemove',
        this.onMouseMove.bind(this)
      );
      this.mouseUpListener = this.renderer.listen(
        'window',
        'mouseup',
        this.onMouseUp.bind(this)
      );
    });
  }

  private onMouseMove(event: MouseEvent): void {
    if (!this.isDragging) return;

    const dialogElement = this.dialogRef.nativeElement;
    dialogElement.style.left = `${event.clientX - this.offsetX}px`;
    dialogElement.style.top = `${event.clientY - this.offsetY}px`;
  }

  private onMouseUp(): void {
    this.isDragging = false;

    if (this.mouseMoveListener) {
      this.mouseMoveListener();
      this.mouseMoveListener = undefined;
    }
    if (this.mouseUpListener) {
      this.mouseUpListener();
      this.mouseUpListener = undefined;
    }
  }
}
