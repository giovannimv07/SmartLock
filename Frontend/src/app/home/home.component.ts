import { Component } from "@angular/core";
import { LockComponent } from "../components/lock/lock.component";

@Component({
    selector: "app-home",
    imports: [LockComponent],
    templateUrl: "./home.component.html",
    styleUrl: "./home.component.css",
})
export class HomeComponent {}
