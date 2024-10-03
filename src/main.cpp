#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>

#include "gate_control.h"

// Initialize Preferences for NVS
Preferences preferences;

// Instantiate gate control object
GateControl gate;

// Buffers for Wi-Fi credentials
char ssid[32] = {0};
char password[64] = {0};

// Create a web server object
WebServer server(80);

// HTML page for Gate Control served by the web server
const char GATE_CONTROL_page[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>

<head>
    <meta charset="UTF-8">
    <title>Դարպասների կառավարում</title>
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <style>
        .container {
            display: flex;
            flex-direction: column;
            padding: 5px;
            width: 320px;
            margin: auto;
        }

        .btn {
            padding: 10px;
            margin-bottom: 20px;
            color: white;
            text-transform: capitalize;
            font-weight: bold;
            background: lightgray;
            font-size: 24px;
            border-radius: 10px;
        }

        .open {
            background: green;
        }

        .pause {
            background: black;
        }

        .close {
            background: red;
        }
    </style>
</head>

<body>
    <h1 style="text-align: center;">Կառավարել դարպասը</h1>
    <div style="text-align: center;">
        <p>Դարպասի ընթացիկ վիճակը</p>
        <p style="font-size:24px"><b id="state">{{state}}</b></p>
    </div>
    <div class="container">
        <button class="btn open" onclick="location.href='/open'">Բացել</button>
        <button class="btn pause" onclick="location.href='/pause'">Դադարեցնել</button>
        <button class="btn close" onclick="location.href='/close'">Փակել</button>
    </div>
    <script>
        function updateStateColor() {
            let stateElement = document.getElementById('state');
            let stateText = stateElement.innerText.toLowerCase();

            if (stateText === 'open') {
                stateElement.style.color = 'green';
                stateElement.innerText = 'Բաց է'
            } else if (stateText === 'paused') {
                stateElement.style.color = 'black';
                stateElement.innerText = 'Դադարեցված'
            } else if (stateText === 'closed') {
                stateElement.style.color = 'red';
                stateElement.innerText = 'Փակ է'
            }
        }
        updateStateColor();
    </script>
</body>

</html>
)=====";

// HTML page for Wi-Fi Configuration served by the web server
const char WIFI_CONFIG_page[] PROGMEM = R"=====(
  <!DOCTYPE html>
<html>

<head>
    <meta charset="UTF-8">
    <title>WiFi կարգավորումներ</title>
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
</head>

<body>
    <h1 style="text-align: center;">WiFi-ի տվյալները թարմացվել են։</h1>
    <form action="/save_wifi" method="post">
        <label for="ssid">Wi-Fi Անվանում․</label>
        <input type="text" id="ssid" name="ssid"><br><br>
        <label for="password">Wi-Fi Գաղտնաբառ․</label>
        <input type="text" id="password" name="password"><br><br>
        <input type="submit" value="Save Wi-Fi">
    </form>
</body>

</html>
)=====";

// HTML page for Gate Control served by the web server
const char WIFI_UPDATE_page[] PROGMEM = R"=====(
  <!DOCTYPE html>
<html>

<head>
    <meta charset="UTF-8">
    <title>WiFi կարգավորումներ</title>
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
</head>

<body style="text-align: center;">
    <h1 >WiFi-ի տվյալները թարմացվել են։</h1>
    <p>
        Էջը թարմացնելուց առաջ, համոզվեք, որ ձեր սարքը միացած է WiFi֊ին՝ թարմացված տվյալներով։
    </p>
    <button onclick="location.href='/';">Թարմացնել էջը</button>
</body>

</html>
)=====";

void saveWiFiCredentials(const String &new_ssid, const String &new_password)
{
    Serial.println("Opening NVS...");
    if (!preferences.begin("wifi_prefs", false))
    {
        Serial.println("Failed to open NVS");
        return;
    }

    Serial.println("Clearing previous credentials...");
    preferences.clear();

    Serial.println("Storing new SSID...");
    preferences.putString("wifi_ssid", new_ssid);

    Serial.println("Storing new password...");
    preferences.putString("wifi_password", new_password);

    Serial.println("Closing NVS...");
    preferences.end();

    Serial.println("Wi-Fi credentials saved.");
}

// Function to securely load Wi-Fi credentials
bool loadWiFiCredentials()
{
    preferences.begin("wifi_prefs", true); // Open NVS in read-only mode

    // preferences.clear();

    // Check if the credentials exist
    if (preferences.isKey("wifi_ssid") && preferences.isKey("wifi_password"))
    {
        String stored_ssid = preferences.getString("wifi_ssid");
        String stored_password = preferences.getString("wifi_password");
        // Ensure that the credentials fit into the buffers
        stored_ssid.toCharArray(ssid, sizeof(ssid));
        stored_password.toCharArray(password, sizeof(password));
        preferences.end(); // Close the preferences
        return true;       // Credentials found and loaded
    }
    else
    {
        preferences.end(); // Close the preferences
        return false;      // Credentials not found
    }
}

// Serve the Gate Control page
void handleGateControl()
{
    String html = "";

    // Check if the URL contains the parameter ?r=true
    if (server.hasArg("r") && server.arg("r") == "true")
    {
        html = WIFI_UPDATE_page;
        server.send(200, "text/html", html);

        delay(2000);
        ESP.restart();
    }
    else
    {
        html = GATE_CONTROL_page;
        html.replace("{{state}}", gate.getStateText());
    }

    // Add no-cache headers
    server.sendHeader("Cache-Control", "no-store, no-cache, must-revalidate, max-age=0");
    server.sendHeader("Pragma", "no-cache");
    server.sendHeader("Expires", "-1");
    server.send(200, "text/html", html);
}

// Serve the Wi-Fi Configuration page
void handleWiFiConfig()
{
    server.send(200, "text/html", WIFI_CONFIG_page);
}

// Save the Wi-Fi credentials and reboot
void handleSaveWifi()
{
    if (server.hasArg("ssid") && server.hasArg("password"))
    {
        String new_ssid = server.arg("ssid");
        String new_password = server.arg("password");

        // Validate the credentials
        if (new_ssid.length() > 0 && new_password.length() >= 8)
        {
            saveWiFiCredentials(new_ssid, new_password);

            server.sendHeader("Location", "/?r=true");
            server.send(303); // 303: See Other (redirect)
        }
        else
        {
            server.send(400, "text/plain", "SSID must not be empty and password must be at least 8 characters.");
        }
    }
    else
    {
        server.send(400, "text/plain", "Missing SSID or password");
    }
}

// Function to start Access Point mode
void startAccessPoint()
{
    Serial.println("Starting Access Point...");
    WiFi.softAP("Senso Lab Technologies", "your_password"); // Set a strong password or allow user-defined password
    Serial.println("Access Point started.");
    Serial.print("IP Address: ");
    Serial.println(WiFi.softAPIP());
}

// Use credentials in your Wi-Fi setup
void setupWiFi()
{
    // preferences.clear();
    if (loadWiFiCredentials())
    {
        // If credentials are loaded, attempt to connect to the Wi-Fi
        WiFi.softAP(ssid, password);
        // WiFi.softAP("Senso Lab Technologies", "your_password");
    }
    else
    {
        saveWiFiCredentials("Senso Lab Technologies", "your_password");
        ESP.restart();
        // startAccessPoint(); // Start Access Point mode if no credentials are available
    }
    Serial.print("Access Point gateway IP: ");
    Serial.println(WiFi.gatewayIP());

    Serial.print("SSID: ");
    Serial.println(ssid);

    Serial.print("Password: ");
    Serial.println(password);
}

void setup()
{
    Serial.begin(115200);

    // Setup GPIO and gate control
    gate.init();

    // Start Wi-Fi Access Point
    setupWiFi();

    // Define routes for the Gate Control and Wi-Fi Configuration pages
    server.on("/", handleGateControl);                  // Main page for gate control
    server.on("/wifi", handleWiFiConfig);               // Separate page for Wi-Fi configuration
    server.on("/save_wifi", HTTP_POST, handleSaveWifi); // Save Wi-Fi settings

    server.on("/open", []()
              { gate.setState(GATE_OPEN); server.sendHeader("Location", "/");
        server.send(303); });
    server.on("/pause", []()
              { gate.setState(GATE_PAUSE); server.sendHeader("Location", "/");
        server.send(303); });
    server.on("/close", []()
              { gate.setState(GATE_CLOSE); server.sendHeader("Location", "/");
        server.send(303); });

    // Start the web server
    server.begin();
    Serial.println("Web server started.");
}

void loop()
{
    server.handleClient(); // Handle web server requests
    gate.update();
}
