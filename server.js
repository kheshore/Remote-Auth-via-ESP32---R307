const express = require("express");
const bodyParser = require("body-parser");
const axios = require("axios");
const path = require("path");
const FormData = require("form-data");

const app = express();
const port = 3000;

app.use(bodyParser.urlencoded({ extended: true }));
app.use(express.static(path.join(__dirname, "public")));

const enrolledUsers = {};

app.get("/", (req, res) => {
  res.sendFile(path.join(__dirname, "public", "index.html")); 
});

app.post("/register", async (req, res) => {
  const { email, id, name } = req.body;

  if (Object.values(enrolledUsers).some(user => user.email === email)) {
    return res
      .status(400)
      .json({ success: false, message: "Email already registered" });
  }

  const formData = new FormData();
  formData.append("id", id);
  try {
    await axios.post("http://192.168.0.119/enroll", formData, {
    headers: {
      "Content-Type": "multipart/form-data",
    },
  });
    res.json({ success: true, message: "Registration successful" });
    enrolledUsers[id] = { email, name };
  } catch (error) {
    console.error("Error enrolling fingerprint"); 
    res
      .status(500)
      .json({ success: false, message: "Error enrolling fingerprint" });
  }
});

app.post("/login", async (req, res) => {
  const { email } = req.body;
  const id = Object.keys(enrolledUsers).find(key => enrolledUsers[key].email === email);

  if (id) {
    const formData = new FormData();
    formData.append("id", id);
    try {
      await axios.post("http://192.168.0.119/verify", formData, {
        headers: {
          "Content-Type": "multipart/form-data",
        },
      });
      res.json({ success: true, message: `Hello, ${enrolledUsers[id].name}` });
    } catch (error) {
      console.error("Error verifying fingerprint:", error.response.data);
      res
        .status(500)
        .json({ success: false, message: error.response.data });
    }
  }
  else if (id === undefined){
    res.status(400).json({ success: false, message: "Empty DB Please Register" });
  } 
  else {
    res.status(400).json({ success: false, message: "Invaild Credentials" });
  }
});

app.listen(port, () => {
  console.log(`Server running at http://localhost:${port}`);
});
