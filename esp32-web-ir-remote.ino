/*  ESP32 Web Infrared Remote
 *  custom app built for fun and controlling my TV&Soundbar pair
 *  with just the phone, or laptop, or anything web-capable
 */

/*  Before building, read this issue
 *  https://github.com/crankyoldgit/IRremoteESP8266/issues/1018
 *  and ensure that IRremoteESP8266 library has this code in ir_Sony.cpp for sendSony
 *  method, changing carrier frequency to 38 KHz because this is what HT-CT380 soundbar
 *  operates on. The changed method should have this code:

    void IRsend::sendSony(uint64_t data, uint16_t nbits, uint16_t repeat) {
      sendGeneric(kSonyHdrMark, kSonySpace, kSonyOneMark, kSonySpace, kSonyZeroMark,
                  kSonySpace,
                  0,  // No Footer mark.
                  kSonyMinGap, kSonyRptLength, data, nbits, 38, true, repeat, 33);
    }

 */


#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRsend.h>
#include <IRutils.h>

#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>

// WLAN data
#include "credentials.h"  // not versioned
const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWD;

// static IP
IPAddress ip(192,168,1,60);
IPAddress gateway(192,168,1,1);
IPAddress subnet(255,255,255,0);

//WifiServer server(80);
WebServer server(80);


// set your pin here
IRsend irsend(15); //an IR led (circuit's transistor base) is connected to pin D15

uint16_t x240crawData[127] = {2448, 550,  648, 544,  1250, 546,  648, 548,  648, 550,  1272, 524,  648, 550,  644, 550,  674, 524,  648, 550,  648, 544,  674, 524,  1270, 524,  1246, 550,  674, 524,  648, 22404,  2474, 524,  674, 520,  1250, 548,  648, 544,  674, 524,  1270, 524,  648, 550,  648, 546,  674, 524,  648, 546,  652, 546,  674, 524,  1270, 524,  1272, 522,  674, 520,  648, 22404,  2452, 544,  674, 524,  1270, 524,  674, 518,  674, 522,  1246, 550,  674, 524,  648, 544,  674, 524,  648, 546,  674, 524,  674, 518,  1276, 518,  1276, 524,  648, 546,  674, 22380,  2474, 520,  674, 524,  1250, 544,  674, 524,  674, 518,  1276, 520,  674, 522,  674, 524,  674, 520,  674, 524,  674, 524,  674, 518,  1276, 518,  1276, 524,  672, 524,  648};  // SONY 240C
uint16_t x640crawData[127] = {2448, 548,  1272, 524,  1270, 524,  648, 550,  644, 550,  1250, 550,  668, 524,  674, 528,  644, 550,  670, 524,  674, 524,  648, 550,  1246, 548,  1270, 524,  674, 524,  648, 21808,  2470, 524,  1250, 544,  1250, 550,  670, 524,  674, 524,  1270, 524,  674, 524,  644, 550,  674, 524,  674, 524,  644, 554,  670, 524,  1270, 524,  1270, 524,  674, 524,  644, 21812,  2470, 524,  1250, 544,  1250, 550,  670, 524,  674, 524,  1270, 524,  648, 550,  670, 524,  674, 524,  648, 548,  670, 524,  674, 524,  1270, 522,  1272, 524,  648, 548,  670, 21784,  2470, 524,  1250, 546,  1250, 550,  670, 524,  674, 518,  1250, 550,  670, 522,  648, 550,  670, 524,  674, 526,  644, 546,  674, 524,  1270, 524,  1246, 548,  674, 524,  648};  // SONY 640C
uint16_t x540crawData[127] = {2444, 550,  1272, 524,  648, 550,  1246, 550,  644, 550,  1246, 554,  670, 524,  648, 550,  670, 524,  648, 550,  644, 554,  670, 524,  1272, 524,  1246, 550,  648, 550,  644, 21810,  2448, 550,  1246, 550,  644, 554,  1242, 554,  644, 550,  1270, 524,  648, 550,  644, 554,  644, 548,  644, 554,  644, 548,  648, 548,  1246, 548,  1246, 550,  644, 554,  644, 21806,  2444, 554,  1240, 554,  644, 550,  1246, 550,  644, 554,  1242, 554,  644, 548,  644, 554,  644, 550,  644, 554,  644, 554,  644, 548,  1246, 548,  1246, 552,  644, 548,  644, 21812,  2418, 574,  1246, 548,  622, 574,  1246, 548,  644, 554,  1240, 554,  644, 554,  644, 548,  644, 552,  644, 554,  644, 550,  644, 554,  1240, 554,  1220, 576,  644, 554,  644};  // SONY 540C
uint16_t x4b0drawData[127] = {2474, 524,  1274, 520,  674, 524,  674, 524,  1274, 520,  674, 524,  1280, 514,  1274, 524,  674, 524,  674, 522,  648, 544,  674, 524,  1276, 518,  1276, 524,  674, 520,  1274, 20590,  2472, 524,  1276, 522,  674, 518,  652, 544,  1276, 522,  674, 518,  1274, 524,  1250, 544,  674, 524,  674, 522,  674, 524,  674, 518,  1274, 524,  1244, 550,  674, 524,  1274, 20584,  2474, 524,  1276, 518,  674, 524,  674, 524,  1270, 524,  674, 524,  1270, 524,  1272, 524,  674, 524,  674, 524,  648, 544,  674, 524,  1250, 544,  1276, 520,  674, 524,  1276, 20584,  2474, 524,  1276, 524,  648, 546,  674, 524,  1250, 544,  674, 524,  1270, 524,  1250, 544,  674, 522,  674, 524,  674, 524,  648, 544,  1252, 542,  1276, 524,  650, 544,  1274};  // SONY 4B0D
uint16_t x140crawData[95] = {2448, 548,  644, 554,  644, 548,  1250, 550,  644, 550,  1248, 550,  648, 550,  644, 554,  644, 550,  648, 548,  648, 548,  648, 548,  1246, 548,  1250, 550,  644, 554,  644, 22414,  2448, 548,  648, 550,  648, 550,  1244, 554,  644, 550,  1250, 550,  644, 550,  648, 550,  648, 550,  644, 554,  644, 550,  648, 550,  1244, 550,  1246, 550,  648, 550,  648, 22412,  2448, 550,  648, 550,  648, 550,  1244, 550,  648, 550,  1244, 550,  648, 550,  648, 550,  648, 550,  644, 550,  648, 550,  648, 550,  1244, 550,  1244, 554,  644, 554,  644};  // SONY 140C
uint16_t x1d0crawData[95] = {2448, 548,  644, 552,  644, 554,  1240, 554,  1246, 550,  1246, 554,  644, 550,  1244, 550,  622, 576,  644, 554,  644, 550,  622, 576,  1246, 550,  1244, 554,  644, 550,  644, 21222,  2444, 550,  622, 576,  644, 554,  1242, 554,  1220, 576,  1244, 550,  622, 576,  1220, 574,  644, 552,  644, 554,  644, 550,  648, 550,  1246, 550,  1246, 548,  622, 576,  644, 21216,  2444, 554,  644, 550,  622, 574,  1244, 550,  1246, 552,  1242, 554,  644, 554,  1220, 574,  644, 554,  644, 550,  648, 550,  644, 554,  1246, 548,  1246, 554,  644, 554,  644};  // SONY 1D0C
uint16_t x5d0crawData[127] = {2448, 550,  1244, 550,  648, 548,  1246, 548,  1246, 548,  1246, 554,  644, 550,  1244, 550,  648, 550,  644, 554,  644, 550,  648, 550,  1246, 548,  1246, 550,  648, 550,  644, 20614,  2444, 554,  1246, 550,  644, 550,  1250, 550,  1246, 548,  1246, 550,  644, 554,  1246, 550,  644, 554,  670, 524,  648, 550,  644, 550,  1246, 550,  1244, 554,  644, 550,  648, 20616,  2470, 528,  1246, 550,  670, 524,  1244, 554,  1244, 550,  1246, 550,  648, 550,  1246, 548,  644, 550,  648, 550,  644, 554,  670, 524,  1246, 554,  1246, 548,  670, 528,  644, 20614,  2444, 554,  1246, 548,  644, 554,  1246, 548,  1246, 550,  1246, 550,  648, 550,  1246, 550,  644, 554,  644, 548,  648, 550,  644, 554,  1246, 550,  1244, 550,  644, 554,  644};  // SONY 5D0C


void handleRoot() {
  server.sendHeader("Access-Control-Max-Age", "10000");
  server.sendHeader("Access-Control-Allow-Methods", "POST,GET,OPTIONS");
  server.sendHeader("Access-Control-Allow-Headers", "Origin, X-Requested-With, Content-Type, Accept");
  
 server.send(200, "text/plain", "Please specify command! Form: /ir?code=xxx&bits=xx&protocol=x");
}

void handleIr(){
  server.sendHeader("Access-Control-Allow-Origin", "*");
  
  String codestring = server.arg("code");
  String protocol = server.arg("protocol");
  String bitsstring = server.arg("bits");
  String webOutput = "Protocol: "+protocol+"; Code: "+codestring+"; Bits: "+bitsstring+"\n";
  if ((codestring != "")&&(bitsstring != "")){
    unsigned long code = codestring.toInt();
    int bits = bitsstring.toInt();
    if (protocol == "NEC"){
      server.send(200, "text/html", webOutput);
      irsend.sendNEC(code, bits);
    }
    else if (protocol == "Sony"){
      server.send(200, "text/html", webOutput);

/*
      // sending raw because: 
      // https://github.com/crankyoldgit/IRremoteESP8266/issues/1018
      
      if(code==9228) //0x240c
        irsend.sendRaw(x240crawData, 127, 38);
      else if(code==21516) //0x540c
        irsend.sendRaw(x540crawData, 127, 38);
      else if(code==25612) //0x640c
        irsend.sendRaw(x640crawData, 127, 38);
      else if(code==19213) //0x4B0D
        irsend.sendRaw(x4b0drawData, 127, 38);
      else if(code==5132) //0x140C
        irsend.sendRaw(x140crawData, 95, 38);
      else if(code==7436) //0x1d0c
        irsend.sendRaw(x1d0crawData, 95, 38);
      else if(code==23820) //0x5d0c
        irsend.sendRaw(x5d0crawData, 127, 38);
  */    

      irsend.sendSony(code, bits, 3);
    }
    else if (protocol == "Whynter"){
      server.send(200, "text/html", webOutput);
      irsend.sendWhynter(code, bits);
    }
    else if (protocol == "LG"){
      server.send(200, "text/html", webOutput);
      irsend.sendLG(code, bits);
    }
    else if (protocol == "RC5"){
      server.send(200, "text/html", webOutput);
      irsend.sendRC5(code, bits);
    }
    else if (protocol == "RC6"){
      server.send(200, "text/html", webOutput);
      irsend.sendRC6(code, bits);
    }
    else if (protocol == "DISH"){
      server.send(200, "text/html", webOutput);
      irsend.sendDISH(code, bits);
    }
    else if (protocol == "SharpRaw"){
      server.send(200, "text/html", webOutput);
      irsend.sendSharpRaw(code, bits);
    }
    else if (protocol == "Samsung"){
      server.send(200, "text/html", webOutput);
      irsend.sendSAMSUNG(code, bits);
    }
    else {
      server.send(200, "text/html", "Protocol not implemented!\n");
    }
  }
  else {
    server.send(200, "text/html", "Missing code or bits!");
  }
}
void handleNotFound(){
  server.send(404, "text/plain", "404 Not Found");
}
void setup(void){
  Serial.begin(115200);
  Serial.println("Hello world, initalizing...");
  irsend.begin();
  irsend.calibrate();
  
  WiFi.begin(ssid, password);
  WiFi.config(ip, gateway, subnet);
  Serial.println("");
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  
  server.on("/", handleRoot);
  server.on("/ir", handleIr);
  server.onNotFound(handleNotFound);
  server.begin();
  Serial.println("HTTP server started");

}
void loop(void){
  server.handleClient();
}
