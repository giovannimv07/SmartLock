import { NgIf } from "@angular/common";
import { Component } from "@angular/core";

@Component({
    selector: "app-lock",
    imports: [NgIf],
    templateUrl: "./lock.component.html",
    styleUrl: "./lock.component.css",
})
export class LockComponent {
    isLocked: boolean = true;

    toggleLock() {
        this.isLocked = !this.isLocked;
    }
}
