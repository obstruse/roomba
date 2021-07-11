#ifndef HTTP_H
#define HTTP_H
#include <ESP8266WebServer.h>

ESP8266WebServer server ( 80 );
#endif

//--------------------------------------------
void sensors() {

  getSensors();
  
  sprintf ( temp,  
"<html>\
  <head>\
    <title>Roomba</title>\
    <style>\
      body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }\
      TD   { text-align: left; }\
    </style>\
  </head>\
  <body>\
    <h1>Sensors</h1>\
  <p>roombaAngle %d</p>\
  <p>roombaLightBumper %02x</p>\
  <p>roombaBumps %02x</p>\
  <table><tr>\
  <td>%d</td> \
  <td>%d</td> \
  <td>%d</td> \
  <td>%d</td> \
  <td>%d</td> \
  <td>%d</td> \
  </tr></table> \
  </body>\
  </html>",

    roombaAngle, roombaLightBumper, roombaBumps,
    bumps[0],bumps[1],bumps[2],bumps[3],bumps[4],bumps[5] 
  );
  
  server.send ( 200, "text/html", temp );
}


//--------------------------------------------
void roiCmd() {

  String inArg;
  String inCmd;
  
  inCmd = server.arg("cmd");
  
  inArg = server.arg("speed");
  int speed = inArg.toInt();
  
  inArg = server.arg("angle");
  int angle = inArg.toInt();
  
  inArg = server.arg("dtime");
  int dtime = inArg.toInt();
  
  if (inCmd == "FULL") {      roomba.start(); Serial.swap(); Serial.write(128); roomba.fullMode(); }
  if (inCmd == "PASSIVE") {   roomba.start(); Serial.swap(); Serial.write(128); }
  if (inCmd == "POWER") {     roomba.power(); }
  if (inCmd == "DOCK") {      roomba.dock(); }

  if (inCmd == "DRIVE") {     driveMotion(speed);  }  // this command makes no sense. driveMotion is a distance (or time). DRIVE not used
  if (inCmd == "DISTANCE") {     driveMotion(dtime);  }
  if (inCmd == "SPEED") {     driveMotionVelocity(speed);  }    // minimum speed is 11
  
  if (inCmd == "TURN") {      roomba.drive(speed,Roomba::DriveInPlaceClockwise); }

  if (inCmd == "STOP") {      stopMotion();  }

  if (inCmd == "BEEP") {
    uint8_t song[] = {62, 12, 66, 12, 69, 12, 74, 36};
    roomba.song(0, song, sizeof(song));
    roomba.playSong(0);
  }
  
  if (inCmd == "LON") {       roomba.leds(0x3f, 255, 255); }
  if (inCmd == "LOFF") {      roomba.leds(ROOMBA_MASK_LED_NONE, 0, 0); }

  if (inCmd == "ANGLE") {     angleMotion(angle); }

  if (inCmd == "BNT") {       BumpnTurn=true; bntSpeed = speed; bntAngle = angle; }
    

  server.sendHeader("Location", String("/"), true);
  server.send ( 302, "text/plain", "");

}

//--------------------------------------------
void handleRoot() {
  int sec = millis() / 1000;
  int min = sec / 60;
  int hr = min / 60;
  
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.sendHeader("Content-Type","text/html",true);
  server.sendHeader("Cache-Control","no-cache");
  server.send(200);

  sprintf ( temp,
"<html>\
  <head>\
    <title>Roomba</title>\
    <style>\
      body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }\
      TD   { text-align: left; }\
    </style>\
  </head>\
  <body>\
    <h1>Roomba</h1>" );
  server.sendContent ( temp );


  sprintf ( temp,
"<form action='/steps'>\
<table border='1' cellpadding='5'>\
<tr><th aligh='center'>Patterns</th></tr>\
<tr><td><table border=0 cellpadding=5>");
    server.sendContent ( temp );

  for ( int i = 1; i < maxPattern; i++) {
    sprintf ( temp,
 "<tr><td><input type='radio' name='steps' value='%d' %s></td><td>%s</td></tr>\n",
 i, stepMode == i ? "checked" : "", Pattern[i].Desc.c_str());
    server.sendContent ( temp );    
  }

  sprintf ( temp,
"</table></td></tr>\
<tr><td> \
 <input type='submit' name='start' value='start'>\
 <input type='submit' name='stop'  value='stop'>\
</td></tr>\
</table>\
</form>");
    server.sendContent ( temp );
  



  sprintf ( temp,    
"<table border=1 cellpadding=5><tr><td>\
<table border='0' cellpadding='5'>\
<tr><th colspan='3'>Commands</th></tr>\
<tr><td colspan='3'><hr></td></tr>\
<tr><td colspan='2'>Full Mode</td><td>   <input type=button value='*' onclick='location.href=\"/roi?cmd=FULL\"'></td></tr>\
<tr><td colspan='2'>Power Off</td><td>   <input type=button value='*' onclick='location.href=\"/roi?cmd=POWER\"'></td></tr>\
<tr><td colspan='2'>Dock</td><td>        <input type=button value='*' onclick='location.href=\"/roi?cmd=DOCK\"'></td></tr>\
<tr><td colspan='3'><hr></td></tr>\
<tr><td colspan='2'>Stop</td><td>        <input type=button value='*' onclick='location.href=\"/roi?cmd=STOP\"'></td></tr>\
<tr><td colspan='3'><hr></td></tr>\
<tr><td colspan='2'>Beep</td><td>         <input type=button value='*' onclick='location.href=\"/roi?cmd=BEEP\"'></td></tr>\
<tr><td colspan='2'>LED On</td><td>       <input type=button value='*' onclick='location.href=\"/roi?cmd=LON\"'></td></tr>\
<tr><td colspan='2'>LED Off</td><td>      <input type=button value='*' onclick='location.href=\"/roi?cmd=LOFF\"'></td></tr>\
<tr><td colspan='3'><hr></td></tr>\
<tr><td>Distance</td><td><input type=number name='dtime'></td><td><input type=button value='*' onclick='location.href=\"/roi?cmd=DISTANCE&dtime=\"+document.getElementsByName(\"dtime\")[0].value;'></td></tr>\
<tr><td>Angle</td><td><input type=number name='angle'></td><td><input type=button value='*' onclick='location.href=\"/roi?cmd=ANGLE&angle=\"+document.getElementsByName(\"angle\")[0].value;'></td></tr>\
<tr><td>Speed</td><td><input type=number name='speed'></td><td><input type=button value='*' onclick='location.href=\"/roi?cmd=SPEED&speed=\"+document.getElementsByName(\"speed\")[0].value;'></td></tr>\
<tr><td colspan='3'><hr></td></tr>\
</table>\
</td></tr></table>");
  server.sendContent ( temp );

  sprintf ( temp,
"<pre>%s</pre>",
    httpMessage );
  server.sendContent ( temp );


   sprintf ( temp,
"<br><p>Uptime: %02d:%02d:%02d</p>\
    <p>%s/%s %s %s</p>\
    </body></html>",
    hr, min % 60, sec % 60,
    pathname, fileName, __DATE__, __TIME__
  );


  server.sendContent ( temp );

  server.sendContent (" ");

}


/*
<tr><td>BNT</td><td><input type=number name='bntSpeed'><input type=number name='bntAngle'></td> \
<td><input type=button value='*' onclick='location.href=\"/roi?cmd=BNT&speed=\"+document.getElementsByName(\"bntSpeed\")[0].value; \
         +\"&angle=\"+'><document.getElementsByName(\"bntAngle\")[0].value;/td></tr> \*/

//--------------------------------------------
void httpSteps() {

  int inArgs = server.args();
  int notReal;
  
  for ( int i = 0; i < inArgs; i++ ) {
    if ( server.argName(i) == "steps" ) {
      stepMode = server.arg(i).toInt();
    }
    if ( server.argName(i) == "start" ) {
      digitalWrite(LED_BUILTIN, LOW);
      rStep = 0; roombaStatus = 0;
      stepsRunning = true;
    }
    if ( server.argName(i) == "stop" ) {
      stopMotion();
    }  
  }
  
  server.sendHeader("Location", String("/"), true);
  server.send ( 302, "text/plain", "");

}

//--------------------------------------------
void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += ( server.method() == HTTP_GET ) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";

  for ( uint8_t i = 0; i < server.args(); i++ ) {
    message += " " + server.argName ( i ) + ": " + server.arg ( i ) + "\n";
  }

  server.send ( 404, "text/plain", message );
}

//--------------------------------------------
//--------------------------------------------
void httpSetup() {

  server.on ( "/", handleRoot );
  server.on ( "/roi", roiCmd );
  server.on ( "/sensors", sensors );
  server.on ( "/steps", httpSteps );
  server.onNotFound ( handleNotFound );
  server.begin();

//  Serial.println("http started");
}
