import { Component } from "@angular/core";
import { LockComponent } from "../components/lock/lock.component";
import { UserComponent } from "../components/user/user.component";
import { LogComponent } from "../components/log/log.component";

@Component({
    selector: "app-home",
    imports: [LockComponent, UserComponent, LogComponent],
    templateUrl: "./home.component.html",
    styleUrl: "./home.component.css",
})
export class HomeComponent {}
