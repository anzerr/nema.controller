#include <Stepper.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>

const char *ssid = "Stepper_Driver_D830";
const char *password = "";

ESP8266WebServer server(80);

IPAddress ip(192, 168, 1, 140);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);

int stepsPerRevolution = 800;
int stepsSpeed = 60;
int running = 0;
int interval = 500;
int stepMove = 1;
Stepper myStepper = Stepper(stepsPerRevolution, D4, D3);;
long previousMillis = 0;

void createStepper(int a) {
    stepsPerRevolution = a;
    myStepper = Stepper(stepsPerRevolution, D4, D3);
    myStepper.setSpeed(stepsSpeed);
}

int setSpeed(int s) {
    stepsSpeed = s;
    myStepper.setSpeed(stepsSpeed);
    return stepsSpeed;
}

void setup() {
    delay(1000);
    Serial.begin(9600);
    Serial.println();

    Serial.print("Configuring access point...");

    Serial.print("Setting soft-AP configuration ... ");
    Serial.println(WiFi.softAPConfig(ip, gateway, subnet) ? "Ready" : "Failed!");

    Serial.print("Setting soft-AP ... ");
    Serial.println(WiFi.softAP(ssid, password) ? "Ready" : "Failed!");

    Serial.print("Soft-AP IP address = ");
    Serial.println(WiFi.softAPIP());

    server.on("/", []() {
        server.send(200, "text/plain", ":)");
    });

    server.on("/driver", []() {
        server.send(200, "text/plain", "TODO");
    });
    server.on("/driver/start", []() {
        running = 1;
        previousMillis = millis();
        server.send(200, "text/plain", String(running));
    });
    server.on("/driver/stop", []() {
        running = 0;
        server.send(200, "text/plain", String(running));
    });
    server.on("/driver/interval", []() {
        if (!server.hasArg("i")) {
            server.send(200, "text/plain", String(interval));
            return;
        }
        interval = server.arg("i").toInt();
        server.send(200, "text/plain", String(interval));
    });
    server.on("/driver/move", []() {
        if (!server.hasArg("i")) {
            server.send(200, "text/plain", String(stepMove));
            return;
        }
        stepMove = server.arg("i").toInt();
        server.send(200, "text/plain", String(stepMove));
    });

    server.on("/driver/steps", []() {
        if (!server.hasArg("i")) {
            server.send(200, "text/plain", String(stepsPerRevolution));
            return;
        }
        createStepper(server.arg("i").toInt());
        server.send(200, "text/plain", String(stepsPerRevolution));
    });

    server.on("/driver/speed", []() {
        if (!server.hasArg("i")) {
            server.send(200, "text/plain", String(stepsSpeed));
            return;
        }
        server.send(200, "text/plain", String(setSpeed(server.arg("i").toInt())));
    });

    server.on("/step", []() {
        if (!server.hasArg("i")) {
            server.send(403, "text/plain", "KO");
            return;
        }
        myStepper.step(server.arg("i").toInt());
        server.send(200, "text/plain", "OK");
    });

    server.begin();
    Serial.println("HTTP server started");

    createStepper(800);
    setSpeed(30);
}

void loop() {
    if (running) {
        unsigned long currentMillis = millis();
        if (currentMillis - previousMillis > interval) {
            previousMillis = currentMillis;
            Serial.println("running move tick");
            myStepper.step(stepMove);
        }
    }
    server.handleClient();
}