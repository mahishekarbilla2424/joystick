#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>

// Replace network credentials with your own WiFi details
const char* ssid = "PROVEINTECH";
const char* password = "Deepu@1507";

bool GPIO_State = 0;
const int Led_Pin = 2;

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

const char html_page[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
    <html>
    <head>
    <title>Arduino - PHPoC Shield</title>
    <meta name="viewport" content="width=device-width, initial-scale=0.7, maximum-scale=0.7">
    <meta charset="utf-8">
    <style>
    body { text-align: center; font-size: width/2pt; }
    h1 { font-weight: bold; font-size: width/2pt; }
    h2 { font-weight: bold; font-size: width/2pt; }
    button { font-weight: bold; font-size: width/2pt; }
    </style>
    <script>
    var canvas_width = 500, canvas_height = 500;
    var radius_base = 150;
    var radius_handle = 72;
    var radius_shaft = 120;
    var range = canvas_width/2 - 10;
    var step = 18;
    var ws;
    var joystick = {x:0, y:0};
    var click_state = 0;
    
    var ratio = 1;
    
    function init()
    {
        var width = window.innerWidth;
        var height = window.innerHeight;
    
        if(width < height)
            ratio = (width - 50) / canvas_width;
        else
            ratio = (height - 50) / canvas_width;
    
        canvas_width = Math.round(canvas_width*ratio);
        canvas_height = Math.round(canvas_height*ratio);
        radius_base = Math.round(radius_base*ratio);
        radius_handle = Math.round(radius_handle*ratio);
        radius_shaft = Math.round(radius_shaft*ratio);
        range = Math.round(range*ratio);
        step = Math.round(step*ratio);
    
        var canvas = document.getElementById("remote");
        //canvas.style.backgroundColor = "#999999";
        canvas.width = canvas_width;
        canvas.height = canvas_height;
    
        canvas.addEventListener("touchstart", mouse_down);
        canvas.addEventListener("touchend", mouse_up);
        canvas.addEventListener("touchmove", mouse_move);
        canvas.addEventListener("mousedown", mouse_down);
        canvas.addEventListener("mouseup", mouse_up);
        canvas.addEventListener("mousemove", mouse_move);
    
        var ctx = canvas.getContext("2d");
        ctx.translate(canvas_width/2, canvas_height/2);
        ctx.shadowBlur = 20;
        ctx.shadowColor = "LightGray";
        ctx.lineCap="round";
        ctx.lineJoin="round";
    
        update_view();
    }
    function connect_onclick()
    {
        if(ws == null)
        {

      var gateway = `ws://${window.location.hostname}/ws`;
            ws = new WebSocket(gateway);
            document.getElementById("ws_state").innerHTML = "CONNECTING";
            ws.onopen = ws_onopen;
            ws.onclose = ws_onclose;
            ws.onmessage = ws_onmessage;
        }
        else
            ws.close();
    }
    function ws_onopen()
    {
        document.getElementById("ws_state").innerHTML = "<font color='blue'>CONNECTED</font>";
        document.getElementById("bt_connect").innerHTML = "Disconnect";
        update_view();
    }
    function ws_onclose()
    {
        document.getElementById("ws_state").innerHTML = "<font color='gray'>CLOSED</font>";
        document.getElementById("bt_connect").innerHTML = "Connect";
        ws.onopen = null;
        ws.onclose = null;
        ws.onmessage = null;
        ws = null;
        update_view();
    }
    function ws_onmessage(e_msg)
    {
        e_msg = e_msg || window.event; // MessageEvent
    
    }
    function send_data()
    {
        var x = joystick.x, y = joystick.y;
        var joystick_range = range - radius_handle;
        x = Math.round(x*100/joystick_range);
        y = Math.round(-(y*100/joystick_range));

        console.log(joystick);
        console.log(joystick_range);
        console.log(x, y);
    
        if(ws != null)
            ws.send(x + ":" + y + "\r\n");
    }
    function update_view()
    {
        var x = joystick.x, y = joystick.y;
    
        var canvas = document.getElementById("remote");
        var ctx = canvas.getContext("2d");
    
        ctx.clearRect(-canvas_width/2, -canvas_height/2, canvas_width, canvas_height);
    
        ctx.lineWidth = 3;
        ctx.strokeStyle="gray";
        ctx.fillStyle = "LightGray";
        ctx.beginPath();
        ctx.arc(0, 0, range, 0, 2 * Math.PI);
        ctx.stroke();
        ctx.fill();
    
        ctx.strokeStyle="black";
        ctx.fillStyle = "hsl(0, 0%, 35%)";
        ctx.beginPath();
        ctx.arc(0, 0, radius_base, 0, 2 * Math.PI);
        ctx.stroke();
        ctx.fill();
    
        ctx.strokeStyle="red";
    
        var lineWidth = radius_shaft;
        var pre_x = pre_y = 0;
        var x_end = x/5;
        var y_end = y/5;
        var max_count  = (radius_shaft - 10)/step;
        var count = 1;
    
        while(lineWidth >= 10)
        {
            var cur_x = Math.round(count * x_end / max_count);
            var cur_y = Math.round(count * y_end / max_count);
            ctx.lineWidth = lineWidth;
            ctx.beginPath();
            ctx.lineTo(pre_x, pre_y);
            ctx.lineTo(cur_x, cur_y);
            ctx.stroke();
    
            lineWidth -= step;
            pre_x = cur_x;
            pre_y = cur_y;
            count++;
        }
    
        var x_start = Math.round(x / 3);
        var y_start = Math.round(y / 3);
        lineWidth += step;
    
        ctx.beginPath();
        ctx.lineTo(pre_x, pre_y);
        ctx.lineTo(x_start, y_start);
        ctx.stroke();
    
        count = 1;
        pre_x = x_start;
        pre_y = y_start;
    
        while(lineWidth < radius_shaft)
        {
            var cur_x = Math.round(x_start + count * (x - x_start) / max_count);
            var cur_y = Math.round(y_start + count * (y - y_start) / max_count);
            ctx.lineWidth = lineWidth;
            ctx.beginPath();
            ctx.lineTo(pre_x, pre_y);
            ctx.lineTo(cur_x, cur_y);
            ctx.stroke();
    
            lineWidth += step;
            pre_x = cur_x;
            pre_y = cur_y;
            count++;
        }
    
        var grd = ctx.createRadialGradient(x, y, 0, x, y, radius_handle);
        for(var i = 85; i >= 50; i-=5)
            grd.addColorStop((85 - i)/35, "hsl(0, 100%, "+ i + "%)");
    
        ctx.fillStyle = grd;
        ctx.beginPath();
        ctx.arc(x, y, radius_handle, 0, 2 * Math.PI);
        ctx.fill();
    }
    function process_event(event)
    {
        var pos_x, pos_y;
        if(event.offsetX)
        {
            pos_x = event.offsetX - canvas_width/2;
            pos_y = event.offsetY - canvas_height/2;
        }
        else if(event.layerX)
        {
            pos_x = event.layerX - canvas_width/2;
            pos_y = event.layerY - canvas_height/2;
        }
        else
        {
            pos_x = (Math.round(event.touches[0].pageX - event.touches[0].target.offsetLeft)) - canvas_width/2;
            pos_y = (Math.round(event.touches[0].pageY - event.touches[0].target.offsetTop)) - canvas_height/2;
        }
    
        return {x:pos_x, y:pos_y}
    }
    function mouse_down()
    {
        if(ws == null)
            return;
    
        event.preventDefault();
    
        var pos = process_event(event);
    
        var delta_x = pos.x - joystick.x;
        var delta_y = pos.y - joystick.y;
    
        var dist = Math.sqrt(delta_x*delta_x + delta_y*delta_y);
    
        if(dist > radius_handle)
            return;
    
        click_state = 1;
    
        var radius = Math.sqrt(pos.x*pos.x + pos.y*pos.y);
        console.log(radius, 'radius');
    
        if(radius <(range - radius_handle))
        {
            joystick = pos;
            send_data();
            update_view();
        }
    }
    function mouse_up()
    {
        event.preventDefault();
        click_state = 0;
    }
    function mouse_move()
    {
        if(ws == null)
            return;
    
        event.preventDefault();
    
        if(!click_state)
            return;
    
        var pos = process_event(event);
    
        var radius = Math.sqrt(pos.x*pos.x + pos.y*pos.y);
    
        if(radius <(range - radius_handle))
        {
            joystick = pos;
            send_data();
            update_view();
        }
    }
    window.onload = init;
    </script>
    </head>
    
    <body>
    
    <p>
    <h1>Arduino - Web-based Joystick</h1>
    </p>
    
    <canvas id="remote"></canvas>
    
    <h2>
    <p>
    WebSocket : <span id="ws_state">null</span>
    </p>
    <button id="bt_connect" type="button" onclick="connect_onclick();">Connect</button>
    </h2>
    
    </body>
    </html>
)rawliteral";

void notifyClients() {
  ws.textAll(String(GPIO_State));
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  Serial.printf("[WSc] get text: %s\n", data);
  //Serial.println((char*)data);
  String data1 = (char*)data;
     if(data1){
         int pos = data1.indexOf(':');
         long x = data1.substring(0, pos).toInt();
         long y = data1.substring(pos+1).toInt();
  
          Serial.print("x:");
           Serial.print(x);
          Serial.print(", y:");
           Serial.println(y);
  
   // scale  from [-100; 100] to [0; 180]
   /*long angle_x = (x + 100) * 180 /200;
   long angle_y = (y + 100) * 180 /200;
   Serial.print(" X: ");
   Serial.println(angle_x);
   Serial.print(" Y: ");
   Serial.println(angle_y);*/
   
     }
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
             void *arg, uint8_t *data, size_t len) {
  switch (type) {
    case WS_EVT_CONNECT:
      Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
      break;
    case WS_EVT_DISCONNECT:
      Serial.printf("WebSocket client #%u disconnected\n", client->id());
      break;
    case WS_EVT_DATA:
      handleWebSocketMessage(arg, data, len);
      break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
      break;
  }
}

void initWebSocket() {
  ws.onEvent(onEvent);
  server.addHandler(&ws);
}

String processor(const String& var){
  Serial.println(var);
  if(var == "STATE"){
    if (GPIO_State){
      return "ON";
    }
    else{
      return "OFF";
    }
  }
}

void setup(){
  // Serial port for debugging purposes
  Serial.begin(115200);

  pinMode(Led_Pin, OUTPUT);
  digitalWrite(Led_Pin, LOW);
  
  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }

  // Print ESP Local IP Address
  Serial.println(WiFi.localIP());

  initWebSocket();

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", html_page, processor);
  });

  // Start server
  server.begin();
}

void loop() {
  ws.cleanupClients();
  digitalWrite(Led_Pin, GPIO_State);
}
