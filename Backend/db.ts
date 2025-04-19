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
	Code: string;
}

export async function checkCode(code: string): Promise<User[]> {
	const [results] = await db.query("SELECT * FROM user WHERE Code = ?", [
		code,
	]);
	return results as User[];
}

export default db;
