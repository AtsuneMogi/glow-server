#include <M5Stack.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include "BLEBeacon.h"

#define BEACON_UUID "65432461-1EFE-4ADB-BC7E-9F7F8E27FDC1"
#define TX_POWER   -65
int MAJOR, MINOR;

BLEAdvertising *pAdvertising;

const char ssid_AP[] = "M5Stack GLOW"; // SSID
const char pass_AP[] = "glowglow"; // password (more than 8 characters)
const IPAddress ip(192, 168, 123, 45);
const IPAddress subnet(255, 255, 255, 0);

String id[] = {"0", "1", "2", "3", "4"};
String color[] = {"0", "448", "56", "7", "22528"};

const char html[] PROGMEM = R"rawliteral(
    <!DOCTYPE html>
    <html lang="en">
        <head>
            <meta charset="UTF-8">
            <meta name="viewport" content="width=device-width, initial-scale=1.0">
            <style type="text/css">
                html {
                    touch-action: manipulation;
                }
                body {
                    text-align: center;
                    font-size: 40px;
                }
                select {
                    width: 200px;
                    height: 100px;
                    font-size: 40px;
                }
                button {
                    width: 200px;
                    height: 100px;
                    font-size: 40px;
                    border-radius: 20px;
                    background: greenyellow;
                }
            </style>
            <title>GLOW</title>
        </head>
        <body>
            <h1>GLOW</h1>
            <form action="/get">
                <hr>ID:
                <select name="id">
                    <option value="0">All</option>
                    <option value="1">1</option>
                    <option value="2">2</option>
                    <option value="3">3</option>
                    <option value="4">4</option>
                </select><br><br>
                <hr>Color:
                <select name="color">
                    <option value="0">Off</option>
                    <option value="448">Red</option>
                    <option value="56">Green</option>
                    <option value="7">Blue</option>
                    <option value="22528">Rainbow</option>
                </select>
                <h1> </h1>
                <button type="submit">
                    Send
                </button>
            </form>
        </body>
    </html>
)rawliteral";

AsyncWebServer server(80); // use port nuber 80


std::string setUUID(){
    std::string org = BEACON_UUID;
    std::string dst = "";
    if (org.length() != 36) {
        return "error";
    }
    dst  = org[34]; dst += org[35];
    dst += org[32]; dst += org[33];
    dst += org[30]; dst += org[31];
    dst += org[28]; dst += org[29];
    dst += org[8];
    dst += org[26]; dst += org[27];
    dst += org[24]; dst += org[25];
    dst += org[23];
    dst += org[21]; dst += org[22];
    dst += org[19]; dst += org[20];
    dst += org[18];
    dst += org[16]; dst += org[17];
    dst += org[14]; dst += org[15];
    dst += org[13];
    dst += org[11]; dst += org[12];
    dst += org[9];  dst += org[10];
    dst += org[6];  dst += org[7];
    dst += org[4];  dst += org[5];
    dst += org[2];  dst += org[3];
    dst += org[0];  dst += org[1];
    return dst;
}


void setBeacon() {
    BLEBeacon oBeacon = BLEBeacon();
    oBeacon.setManufacturerId(0x4C00);
    oBeacon.setProximityUUID(BLEUUID(setUUID()));
    oBeacon.setMajor(MAJOR);
    oBeacon.setMinor(MINOR);
    oBeacon.setSignalPower(TX_POWER);
    BLEAdvertisementData oAdvertisementData = BLEAdvertisementData();
    BLEAdvertisementData oScanResponseData = BLEAdvertisementData();

    oAdvertisementData.setFlags(0x04);

    std::string strServiceData = "";

    strServiceData += (char)26;     // Len
    strServiceData += (char)0xFF;   // Type
    strServiceData += oBeacon.getData(); 
    oAdvertisementData.addData(strServiceData);

    pAdvertising->setAdvertisementData(oAdvertisementData);
    pAdvertising->setScanResponseData(oScanResponseData);
}


void freq(int f, int t) {
    M5.Speaker.tone(f, t);
    delay(t);
    M5.Speaker.mute();
    delay(t);
}


bool play = false;
bool played = false;

void setup() {
    // start M5Stack
    M5.begin();
    // start WiFi access point
    WiFi.softAP(ssid_AP, pass_AP);
    delay(100);                        
    WiFi.softAPConfig(ip, ip, subnet); // set IP address, Gateway, and Subnet
    // set IP address
    IPAddress myIP = WiFi.softAPIP();
    // start async web server
    server.on("/", HTTP_GET, [] (AsyncWebServerRequest *request) {
        request->send(200, "text/html", html);
    });
    server.on("/get", HTTP_GET, [] (AsyncWebServerRequest *request) {
        MAJOR = request->getParam("id")->value().toInt();
        MINOR = 0;
        setBeacon();
        pAdvertising->start();
        delay(200);
        MINOR = request->getParam("color")->value().toInt();
        setBeacon();
        pAdvertising->start();
        delay(100);
        request->send(200, "text/html", html);
    });
    server.begin();
    // print strings on display
    M5.Lcd.printf("SSID: %s\n", ssid_AP);
    M5.Lcd.printf("Password: %s\n", pass_AP);
    M5.Lcd.println("IP address: 192.168.123.45");
    M5.Lcd.qrcode(myIP.toString(), 160, 0, 160, 3);

    BLEDevice::init("");
    pAdvertising = BLEDevice::getAdvertising();

    setBeacon();
    M5.Speaker.setVolume(1);
}


void loop() {
    M5.update();
    if (M5.BtnA.wasPressed()) {
        freq(740, 150);
        freq(587, 150);
        freq(440, 150);
        freq(587, 150);
        freq(659, 150);
        freq(880, 300);
        freq(440, 150);
        freq(659, 150);
        freq(740, 150);
        freq(650, 150);
        freq(440, 150);
        freq(587, 450);
    }
    if (M5.BtnB.wasPressed()) {
        play = true;
    }
    if (play) {
        M5.update();
        if (M5.BtnB.wasPressed()) {
            play = false;
        }
        freq(880, 100); // la
        freq(880, 200); // la
        freq(988, 100);
        freq(880, 100);
        freq(740, 100);
        freq(880, 200);
        
        freq(880, 100); // la
        freq(880, 200); // la
        freq(988, 100);
        freq(880, 100);
        freq(740, 100);
        freq(880, 200);

        freq(587, 100);
        freq(587, 100);
        freq(587, 100);
        freq(659, 100);
        freq(740, 300);
        freq(587, 100);
        freq(740, 300);
        freq(880, 100);
        freq(880, 400);
        
        freq(587, 100);
        freq(587, 100);
        freq(587, 100);
        freq(659, 100);
        freq(740, 400);
        freq(587, 100);
        freq(587, 100);
        freq(587, 100);
        freq(659, 100);
        freq(740, 400);

        freq(659, 100);
        freq(659, 100);
        freq(659, 100);
        freq(587, 100);
        freq(659, 200);
        freq(740, 200);
        freq(880, 200);
        freq(784, 200);
        freq(740, 200);
        freq(659, 200);
        played = true;
    } else if (!play && played) {
        freq(880, 100); // la
        freq(880, 200); // la
        freq(988, 100);
        freq(880, 100);
        freq(740, 100);
        freq(880, 200);
        
        freq(880, 100); // la
        freq(880, 200); // la
        freq(988, 100);
        freq(880, 100);
        freq(740, 100);
        freq(659, 200);
        freq(587, 400);
        played = false;
    }
}