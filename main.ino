#include <WiFi.h>
#include <WebServer.h>
#include <Adafruit_Fingerprint.h>

// Define your Wi-Fi credentials
const char* ssid = "userSSID";
const char* password = "userPassword";

// Define the server port
const int serverPort = 80;

// Maximum number of fingerprints
const int maxFingerprints = 100;

// Array to store enrolled fingerprint IDs
int enrolledIDs[maxFingerprints];
int numEnrolled = 0;

// Create an instance of the web server
WebServer server(serverPort);

// Define the hardware serial port
HardwareSerial serialPort(2); // UART2

// Initialize the fingerprint sensor
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&serialPort);

// Custom error code for when the fingerprint ID does not match the provided ID
#define FINGERPRINT_ID_MISMATCH 0xFF

void setup() {
  Serial.begin(9600);
  
  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  // Start the server
  server.begin();
  Serial.println("Server started");

  // Initialize the fingerprint sensor
  finger.begin(57600);
  
  // Define routes
  server.on("/enroll", HTTP_POST, handleEnroll);
  server.on("/verify", HTTP_POST, handleVerify);
  server.on("/clear", HTTP_GET, handleClear);
  server.on("/list", HTTP_GET, handleList);


}

void loop() {
  // Handle client requests
  server.handleClient();
}

// Handle POST request for enrolling fingerprint
void handleEnroll() {
  if (server.method() == HTTP_POST) {
    // Get the ID from the request body or params
    String id = "";
    if (server.hasArg("id")) {
      id = server.arg("id");
    } else {
      server.send(400, "text/plain", "ID is required");
      return;
    }

    // Attempt to enroll fingerprint for the provided ID
    int enrollResult = getFingerprintEnroll(id.toInt());

    if (enrollResult == FINGERPRINT_OK) {
      server.send(200, "text/plain", "Fingerprint enrolled successfully");
    } else if (enrollResult == FINGERPRINT_PACKETRECIEVEERR || enrollResult == FINGERPRINT_ENROLLMISMATCH) {
      // If the enrollment fails due to mismatch, prompt the user to try again
      if (enrollResult == FINGERPRINT_ENROLLMISMATCH) {
        server.send(500, "text/plain", "Error enrolling fingerprint. Please try again.");
      } else {
        server.send(500, "text/plain", "Error enrolling fingerprint. Please try again. Make sure to capture your fingerprint twice.");
      }
    } else {
      server.send(500, "text/plain", "Unknown error occurred while enrolling fingerprint.");
    }
  } else {
    server.send(405, "text/plain", "Method Not Allowed");
  }
}

// Handle POST request for verifying fingerprint
void handleVerify() {
  if (server.method() == HTTP_POST) {
    // Get the ID from the request body or params
    String id = "";
    if (server.hasArg("id")) {
      id = server.arg("id");
    } else {
      server.send(400, "text/plain", "ID is required");
      return;
    }

    // Verify fingerprint for the provided ID
    int verifyResult = verifyFingerprint(id.toInt());

    if (verifyResult == FINGERPRINT_OK) {
      server.send(200, "text/plain", "Fingerprint verified successfully");
    } else if (verifyResult == FINGERPRINT_PACKETRECIEVEERR || verifyResult == FINGERPRINT_ENROLLMISMATCH || verifyResult == FINGERPRINT_ID_MISMATCH) {
      server.send(400, "text/plain", "Invalid fingerprint");
    } else {
      server.send(500, "text/plain", "Invalid fingerprint / Try Again");
    }
  } else {
    server.send(405, "text/plain", "Method Not Allowed");
  }
}

// Handle GET request to clear stored fingerprints
void handleClear() {
  // Clear enrolled fingerprint IDs array
  numEnrolled = 0;
  server.send(200, "text/plain", "All stored fingerprints cleared successfully");
}

// Handle GET request to list enrolled fingerprints
void handleList() {
  String response = "Enrolled fingerprints:\n";
  for (int i = 0; i < numEnrolled; i++) {
    response += String(enrolledIDs[i]) + "\n";
  }
  server.send(200, "text/plain", response);
}


// Function to enroll fingerprint
int getFingerprintEnroll(int id) {
  int p = -1;
  Serial.print("Waiting for valid finger to enroll as #");
  Serial.println(id);
  for (int i = 0; i < 2; i++) { // Capture fingerprint twice
    while (p != FINGERPRINT_OK) {
      p = finger.getImage();
      switch (p) {
        case FINGERPRINT_OK:
          Serial.println("Image taken");
          break;
        case FINGERPRINT_NOFINGER:
          delay(50);
          break;
        default:
          Serial.println("Unknown error");
          return p;
      }
    }

    p = finger.image2Tz(i + 1);
    if (p != FINGERPRINT_OK) {
      Serial.println("Error converting image");
      return p;
    }
  }

  p = finger.createModel();
  if (p != FINGERPRINT_OK) {
    Serial.println("Error creating model");
    return p;
  }

  p = finger.storeModel(id);
  if (p != FINGERPRINT_OK) {
    Serial.println("Error storing model");
    return p;
  }

  return FINGERPRINT_OK;
}

// Function to verify fingerprint
int verifyFingerprint(int id) {
  int p = -1;
  Serial.println("Waiting for valid finger...");

  while (p != FINGERPRINT_OK) {
    p = finger.getImage();

    switch (p) {
      case FINGERPRINT_OK:
        Serial.println("Image taken");
        break;
      case FINGERPRINT_NOFINGER:
        delay(50);
        break;
      default:
        Serial.println("Unknown error");
        return p;
    }
  }

  p = finger.image2Tz(1);
  if (p != FINGERPRINT_OK) {
    Serial.println("Error converting image");
    return p;
  }

  p = finger.fingerFastSearch();
  if (p != FINGERPRINT_OK) {
    Serial.println("Fingerprint not found");
    return p;
  }

  // Check if the ID matches
  if (finger.fingerID == id) {
    Serial.println("Fingerprint matched");
    return FINGERPRINT_OK;
  } else {
    Serial.println("Fingerprint did not match the provided ID");
    return FINGERPRINT_ID_MISMATCH;
  }
}
