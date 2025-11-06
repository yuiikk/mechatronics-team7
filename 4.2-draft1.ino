//4.2 mobile base control
//draft 1
//team7: yui, boru, qingyun
//make sure upload speed is 115200

#include <WiFi.h>
#include <WebServer.h>

//~~~wifi config~~~
//use at home
const char* SSID     = "MoXianBao";
const char* PASSWORD = "olivedog";
/* use at lab
uncomment this block
    AND in void setup()
    uncomment-> WiFi.config(localIP, gateway, subnet);

const char* SSID     = "TP-Link_8A8C";
const char* PASSWORD = "12488674";
IPAddress localIP(192, 168, 1, 120);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);
*/



//~~~motor pin setup~~~
// left motor (motor A)
const int ENA  = 4;   // PWM enable A
const int IN1  = 5;   // direction 1
const int IN2  = 6;   // direction 2
// right motor (motor B)
const int ENB  = 7;   // PWM enable B
const int IN3  = 8;   // direction 1
const int IN4  = 9;   // direction 2

int speedValue = 180;   // choose between 0-255




//~~~motor helper functions~~~
void stopMotors() {
  digitalWrite(IN1, LOW); digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW); digitalWrite(IN4, LOW);
}

void moveForward() {
  digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
}

void moveReverse() {
  digitalWrite(IN1, LOW); digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW); digitalWrite(IN4, HIGH);
}


//~~~web server~~~ 
WebServer server(80);

//~~~html~~~
const char webpage[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Mobile Base Control</title>
  <style>
    body {
      background: #ffffff;
      font-family: "Poppins", sans-serif;
      display: flex;
      justify-content: center;
      align-items: center;
      height: 100vh;
      margin: 0;
    }

    .container {
      display: flex;
      align-items: center;
      justify-content: center;
    }

    .control-card {
      background: #ffffff;
      border-radius: 18px;
      box-shadow: 0 8px 20px rgba(0,0,0,0.12);
      padding: 40px 30px;
      width: 320px;
      text-align: center;
      position: relative;
    }

    h2 {
      color: #5f8d4e;
      margin-bottom: 20px;
      font-size: 26px;
      font-weight: 700;
    }

    .speed-section p {
      color: #5f8d4e;
      font-weight: 600;
      font-size: 14px;
      margin-bottom: 10px;
    }

    #speedValue {
      font-weight: 800;
      font-size: 16px;
      color: #4b7042;
    }

    .slider {
      width: 80%;
      accent-color: #7fa87f;
      -webkit-appearance: none;
      border: none;
      background: linear-gradient(to right, #8fba8f 0%, #8fba8f var(--value, 40%), #d8e8d8 var(--value, 40%), #d8e8d8 100%);
      height: 6px;
      border-radius: 4px;
      outline: none;
    }

    .slider::-webkit-slider-thumb {
      -webkit-appearance: none;
      appearance: none;
      width: 18px;
      height: 18px;
      border-radius: 50%;
      background: #5f8d4e;
      cursor: pointer;
      border: none;
    }

    .slider::-moz-range-thumb {
      width: 18px;
      height: 18px;
      border-radius: 50%;
      background: #5f8d4e;
      cursor: pointer;
      border: none;
    }

    .control-pad {
      display: flex;
      flex-direction: column;
      align-items: center;
      margin-top: 30px;
    }

    .btn {
      background: #8fba8f;
      border: none;
      border-radius: 10px;
      color: white;
      font-weight: 700;
      font-size: 14px;
      padding: 12px;
      width: 50px;
      height: 50px;
      margin: 6px;
      cursor: pointer;
      transition: all 0.15s ease;
    }

    .btn:hover, .btn.active {
      background: #5f8d4e;
    }

    .center {
      background: #d8e8d8;
      border-radius: 14px;
      position: relative;
    }

    .triangle {
      width: 0;
      height: 0;
      border-left: 10px solid #5f8d4e;
      border-top: 6px solid transparent;
      border-bottom: 6px solid transparent;
      display: inline-block;
    }

    .instructions {
      margin-top: 35px;
      font-size: 11px;
      color: #c2c2c2;
      text-align: center;
      line-height: 1.4;
    }

    .instruction-row {
      display: flex;
      justify-content: space-between;
      margin: 2px 0;
    }
  </style>
</head>
<body tabindex="0">
  <div class="container">
    <div class="control-card">
      <h2>Mobile Base Control</h2>

      <div class="speed-section">
        <p>Speed: <span id="speedValue"><strong>40%</strong></span></p>
        <input type="range" min="0" max="100" value="40" class="slider" id="speedSlider">
      </div>

      <div class="control-pad">
        <button class="btn up" id="btnUp" onclick="fetch('/forward')"><strong>Fw</strong></button>
        <div class="middle-row">
          <button class="btn left" id="btnLeft" onclick="fetch('/left')"><strong>L</strong></button>
          <button class="btn center" id="btnCenter" onclick="fetch('/stop')">
            <span class="triangle"></span>
          </button>
          <button class="btn right" id="btnRight" onclick="fetch('/right')"><strong>R</strong></button>
        </div>
        <button class="btn down" id="btnDown" onclick="fetch('/reverse')"><strong>Rv</strong></button>
      </div>

      <div class="instructions">
        <div class="instruction-row">
          <span>[ ] — Adjust speed</span>
          <span>↑ / ↓ — Forward / Reverse</span>
        </div>
        <div class="instruction-row">
          <span>Space — Brake</span>
          <span>← / → — Turn Left / Right</span>
        </div>
      </div>
    </div>
  </div>

  <script>
    const slider = document.getElementById('speedSlider');
    const speedValue = document.getElementById('speedValue');

    function updateSliderGradient(value) {
      const percentage = value + '%';
      slider.style.setProperty('--value', percentage);
      speedValue.innerHTML = `<strong>${value}%</strong>`;
    }

    updateSliderGradient(slider.value);

    slider.addEventListener('input', () => {
      updateSliderGradient(slider.value);
      fetch(`/speed?value=${slider.value}`);  // send to ESP32
    });

    const buttons = {
      ArrowUp: document.getElementById('btnUp'),
      ArrowDown: document.getElementById('btnDown'),
      ArrowLeft: document.getElementById('btnLeft'),
      ArrowRight: document.getElementById('btnRight'),
      ' ': document.getElementById('btnCenter')
    };

    // Make sure keys always work even if focus lost
    window.addEventListener('keydown', (e) => {
      if (buttons[e.key]) {
        e.preventDefault();
        buttons[e.key].classList.add('active');
        buttons[e.key].click();
      } else if (e.key === '[' || e.key === ']') {
        e.preventDefault();
        let delta = (e.key === '[') ? -5 : 5;
        slider.value = Math.min(100, Math.max(0, parseInt(slider.value) + delta));
        updateSliderGradient(slider.value);
        fetch(`/speed?value=${slider.value}`);

      }
    });

    window.addEventListener('keyup', (e) => {
      if (buttons[e.key]) {
        buttons[e.key].classList.remove('active');
      }
    });
  </script>
</body>
</html>
)rawliteral";
//~html~end~here~


//~~~web handler~~~
void handleRoot() {
  server.send(200, "text/html", webpage);
}

void setup() {
  Serial.begin(115200);

  //uncomment me when USE AT LAB
  //WiFi.config(localIP, gateway, subnet);

  WiFi.begin(SSID, PASSWORD);

  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("Connected! IP: ");
  Serial.println(WiFi.localIP());

  // motor setup
  pinMode(ENA, OUTPUT); pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT);
  pinMode(ENB, OUTPUT); pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT);

  //~~~~~~~~~~~~~~~~~~routes~~~~~~~~~~~~~~~~~~~~~~
  server.on("/", handleRoot);

  //forward 前进
  server.on("/forward", []() {
    Serial.println("Move Forward");
    //我先写前进和后退试试，左转和右转待会儿再说
    moveForward(); 
    analogWrite(ENA, speedValue);
    analogWrite(ENB, speedValue);

    server.send(200, "text/plain", "OK");
  });

  //reverse 后退
  server.on("/reverse", []() {
    Serial.println("Move Reverse");
    moveReverse();
    analogWrite(ENA, speedValue);
    analogWrite(ENB, speedValue);
    server.send(200, "text/plain", "OK");
  });

  //breake 停车
  server.on("/stop", []() {
    Serial.println("Stop / Brake");
    stopMotors();
    server.send(200, "text/plain", "OK");
  });

  //left 左转
  server.on("/left", []() {
    Serial.println("Turn Left");
    server.send(200, "text/plain", "OK");
  });

  //right 右转
  server.on("/right", []() {
    Serial.println("Turn Right");
    server.send(200, "text/plain", "OK");
  });

  server.on("/speed", []() {
    if (server.hasArg("value")) {
      String speed = server.arg("value");
      Serial.print("Speed set to: ");
      Serial.print(speed);
      Serial.println("%");
    }
    server.send(200, "text/plain", "OK");
  });
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


  server.begin();
  Serial.println("Web server started.");
}

void loop() {
  server.handleClient();
}
