import { inject, Injectable } from "@angular/core";
import { HttpClient } from "@angular/common/http";
import { User } from "../interfaces/user";

@Injectable({
    providedIn: "root",
})
export class UserService {
    private http = inject(HttpClient);
    addUser(user: User) {
        const url = "http://localhost:5000/user";
        return this.http.post(url, user);
    }
}
