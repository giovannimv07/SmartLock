import dotenv from "dotenv";
import express from "express";
import cors from "cors";
import { checkCode, addUser } from "./db";

dotenv.config();
const app = express();
const PORT = process.env.PORT || 5000;

app.use(express.json());
app.use(cors());

app.post("/checkCode", async (req, res) => {
	const { code } = req.body;

	try {
		const isCodeValid = await checkCode(code);
		console.log("Message:", isCodeValid[0]);
		if (isCodeValid.length > 0) {
			res.status(200).json({ message: "Code is valid" });
		} else {
			res.status(404).json({ message: "Code is invalid" });
		}
	} catch (error) {
		console.error("Error checking code:", error);
		res.status(500).json({ message: "Error checking code" });
	}
});

app.post("/user", async (req, res) => {
	const { Name, LastName, Code } = req.body;
	try {
		const user = await addUser(req.body);
		if (user.affectedRows > 0) {
			res.status(200).json({
				message: "User added successfully",
				userId: user.insertId,
			});
		} else {
			res.status(400).json({ message: "User not added" });
		}
	} catch (error: any) {
		if (error.message === "Code already exists") {
			res.status(409).json({ message: "Code already exists" });
		} else {
			console.error("Error adding user:", error);
			res.status(500).json({ message: "Error adding user" });
		}
	}
});

app.listen(PORT, () => {
	console.log(`Server is running on http://localhost:${PORT}`);
});
