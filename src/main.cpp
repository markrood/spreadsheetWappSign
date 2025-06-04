#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "mbedtls/sha256.h"
#include <FirebaseESP32.h>
#include "firebdb.h"


const char* ssid = "Mark";
const char* password = "Frinov25";
const char* scriptURL = "https://gist.githubusercontent.com/markrood/0daf51e2233a90464332a5bccda7767b/raw/115e2ccbee1f0c5ed3494eb3fc118da94421c265/wapp24";

// For the following credentials, see examples/Authentications/SignInAsUser/EmailPassword/EmailPassword.ino

/* 2. Define the API Key */
#define API_KEY "AIzaSyA9Tb-xbeCJzh9pAVq64kFP_mcwA4sbp-g"

/* 3. Define the RTDB URL */
#define DATABASE_URL "https://esp32-firebase-example-852b0-default-rtdb.firebaseio.com/" 

/* 4. Define the user Email and password that alreadey registerd or added in your project */
#define USER_EMAIL "mmkkrood@aol.com"
#define USER_PASSWORD "Skippy"

        FirebaseData fbdo;

        FirebaseAuth auth;
        FirebaseConfig config;


String title = "";
String artist = "";
String who = "";

Firebdb *fb;

String generateUniqueKey(const String* strings, size_t count);
long generateHash(String str);

void fetchAndParseJson(const char* url) {
  HTTPClient http;
  http.begin(url);
  http.setUserAgent("ESP32");

  int httpCode = http.GET();
  Serial.printf("HTTP Code: %d\n", httpCode);

  // Handle redirect manually
  if (httpCode == 302 || httpCode == 301) {
    String newLocation = http.header("Location");
    Serial.println("Redirected to:");
    Serial.println(newLocation);
    http.end();

    // Follow the redirect
    http.begin(newLocation);
    httpCode = http.GET();
    Serial.printf("Redirected HTTP Code: %d\n", httpCode);
  }

  if (httpCode > 0) {
    String payload = http.getString();
    Serial.println("Payload:");
    Serial.println(payload);

    const size_t capacity = 8192;
    DynamicJsonDocument doc(capacity);

    DeserializationError error = deserializeJson(doc, payload);
    if (!error) {
      for (JsonObject row : doc.as<JsonArray>()) {
        title = row["Title"] | "N/A";
        artist = row["Artist"] | "N/A";
        who = row["Who"] | "N/A";
        Serial.printf("Title: %s | Artist: %s | Who: %s\n", title.c_str(), artist.c_str(), who.c_str());
          const String titleArtist = title+" "+artist;
  long oldHash = generateHash(titleArtist);
  String strOldHash = String(oldHash);
  String na =fb->getJustName(strOldHash);
  Serial.println("name is "+na);
  String yr = fb->getJustYear(strOldHash);
  Serial.println("year is "+yr);
  String hash = generateUniqueKey(&titleArtist, 1);
  Serial.println("Generated Hash: "  +hash);
  //fb->writeKeyToDb(hash, na, yr);
  delay(100);
      }
    } else {
      Serial.print("JSON parsing error: ");
      Serial.println(error.c_str());
    }
  } else {
    Serial.printf("HTTP error: %d\n", httpCode);
  }

  http.end();
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected!");
      Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);

  /* Assign the api key (required) */
  config.api_key = API_KEY;

  /* Assign the user sign in credentials */
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;
  /* Assign the callback function for the long running token generation task */
  //config.token_status_callback = tokenStatusCallback; // see addons/TokenHelper.h

  // Comment or pass false value when WiFi reconnection will control by your code or third party library e.g. WiFiManager
  Firebase.reconnectNetwork(true);

  // Since v4.4.x, BearSSL engine was used, the SSL buffer need to be set.
  // Large data transmission may require larger RX buffer, otherwise connection issue or data read time out can be occurred.
  fbdo.setBSSLBufferSize(4096 /* Rx buffer size in bytes from 512 - 16384 */, 1024 /* Tx buffer size in bytes from 512 - 16384 */);

  // Or use legacy authenticate method
  // config.database_url = DATABASE_URL;
  // config.signer.tokens.legacy_token = "<database secret>";

  // To connect without auth in Test Mode, see Authentications/TestMode/TestMode.ino

  //////////////////////////////////////////////////////////////////////////////////////////////
  // Please make sure the device free Heap is not lower than 80 k for ESP32 and 10 k for ESP8266,
  // otherwise the SSL connection will fail.
  //////////////////////////////////////////////////////////////////////////////////////////////
 Firebase.begin(&config, &auth);

  Firebase.setDoubleDigits(5);
   Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);


  fb = new Firebdb();
  fb->addFB(new FirebaseData());

  fetchAndParseJson(scriptURL);

}

void loop() {
  // No loop logic
}

String generateUniqueKey(const String* strings, size_t count) {
  mbedtls_sha256_context ctx;
  mbedtls_sha256_init(&ctx);
  mbedtls_sha256_starts_ret(&ctx, 0);  // 0 = SHA-256 (not SHA-224)

  for (size_t i = 0; i < count; ++i) {
    const char* str = strings[i].c_str();
    size_t len = strings[i].length();
    mbedtls_sha256_update_ret(&ctx, (const unsigned char*)str, len);
  }

  unsigned char hash[32];
  mbedtls_sha256_finish_ret(&ctx, hash);
  mbedtls_sha256_free(&ctx);

  // Convert hash to hex string
  String result = "";
  for (int i = 0; i < 32; i++) {
    if (hash[i] < 16) result += "0";
    result += String(hash[i], HEX);
  }

  return result;
}

long generateHash(String str){
long result = 0;
for (int i = 0; i < str.length(); i++){
   result += pow(27, 15 - i - 1)*(1 + str.charAt(i) - 'a');
}
   return result;
}