import mysql from "mysql2/promise";
import dotenv from "dotenv";
dotenv.config();

// Database Section
const db = mysql.createPool({
	host: process.env.DB_HOST,
	user: process.env.DB_USER,
	password: process.env.DB_PASSWORD,
	database: process.env.DB_NAME,
});

interface User {
	Id: number;
	Name: string;
	LastName: string;
	Code: number;
}

export async function checkCode(code: number): Promise<User[]> {
	try {
		const [results] = await db.query("SELECT * FROM user WHERE Code = ?", [
			code,
		]);
		return results as User[];
	} catch (error: any) {
		console.error("Error checking code in database:", error);
		throw new Error("Error checking code");
	}
}
export async function addUser(
	user: User
): Promise<{ insertId: number; affectedRows: number }> {
	const { Name, LastName, Code } = user;
	try {
		const [res]: any = await db.query(
			"INSERT INTO user (Name, LastName, Code) VALUES (?, ?, ?)",
			[Name, LastName, Code]
		);
		console.log("User added db:", user);
		return { insertId: res.insertId, affectedRows: res.affectedRows };
	} catch (error: any) {
		if (error.code === "ER_DUP_ENTRY") {
			throw new Error("Code already exists");
		}
		throw error;
	}
}

export default db;
