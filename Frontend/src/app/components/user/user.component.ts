import { Component, inject } from "@angular/core";
import { UserService } from "../../services/user.service";
import { User } from "../../interfaces/user";
import { FormsModule } from "@angular/forms";
import { NgIf } from "@angular/common";

@Component({
    selector: "app-user",
    imports: [FormsModule, NgIf],
    templateUrl: "./user.component.html",
    styleUrl: "./user.component.css",
})
export class UserComponent {
    userService = inject(UserService);
    errorMessage: string = "";
    onSubmit(user: User) {
        if (!user.Name || !user.LastName || !user.Code) {
            this.errorMessage = "All fields are required";
            return;
        }

        const codePattern = /^\d{4}$/;
        user.Code = Number(user.Code);
        if (!codePattern.test(String(user.Code))) {
            this.errorMessage = "Code must be a 4-digit code";
            return;
        }
        this.errorMessage = "";
        // console.log("User data:", user);
        this.userService.addUser(user).subscribe({
            next: (res: any) => {
                console.log("User added successfully:", res);
            },
            error: (error) => {
                if (error.status === 409) {
                    this.errorMessage = "A user with this code already exists.";
                } else {
                    this.errorMessage =
                        error.error.message || "An unexpected error occurred.";
                }
            },
        });
    }
}
