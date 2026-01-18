import { ComponentFixture, TestBed } from '@angular/core/testing';

import { TextareaTwoComponent } from './textarea-two.component';

describe('TextareaTwoComponent', () => {
  let component: TextareaTwoComponent;
  let fixture: ComponentFixture<TextareaTwoComponent>;

  beforeEach(async () => {
    await TestBed.configureTestingModule({
      imports: [TextareaTwoComponent]
    })
    .compileComponents();

    fixture = TestBed.createComponent(TextareaTwoComponent);
    component = fixture.componentInstance;
    fixture.componentRef.setInput('inputPlaceholder', 'Placeholder');
    fixture.componentRef.setInput('inputChange', (_inputStr:string)=>'outputStr');
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
