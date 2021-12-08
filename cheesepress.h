#ifndef _CMAP_H
#define _CMAP_H

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <title>Cheese Press Controller </title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="icon" href="data:,">
  <style>
  html {
    font-family: Arial, Helvetica, sans-serif;
    text-align: center;
  }
  h1 {
    font-size: 1.8rem;
    color: white;
  }
  h2{
    font-size: 1.5rem;
    font-weight: bold;
    color: #143642;
  }
  .topnav {
    overflow: hidden;
    background-color: #143642;
  }
  body {
    margin: 0;
  }
.slidecontainer {
  width: 50OFF;
  height: 30px;
  border-radius: 5px;  
  background: #d3d3d3;
  outline: none;
  opacity: 0.7;
  -webkit-transition: .2s;
  transition: opacity .2s;
}

.slider::-webkit-slider-thumb {
  -webkit-appearance: none;
  appearance: none;
  width: 50px;
  height: 100px;
  border-radius: 50OFF;
  background: #4CAF50;
  cursor: pointer;
}

.press {
  background-color: lightgreen; /* Green */
/*
  border: 5;
  width: 200px;
  height: 100px;
 
  color: white;
  padding: 15px 32px;
  text-align: center;
  text-decoration: none;
*/
  border-radius: 20px;
  font-size: 30px; 
  margin-bottom: 1cm;
  margin-top: 1cm; 
}

.release {
  background-color: #FF3C2F; /* Green */
  border: none;
  color: white;
  padding: 15px 32px;
  text-align: center;
  text-decoration: none;
  border-radius: 20px;
  font-size: 40px;
  cursor: pointer; /* Pointer/hand icon */
}

/* Clear floats (clearfix hack) */
.btn-group:after {
  content: "";
  clear: both;
  display: table;
}

.btn-group button {
  background-color: #04AA6D; /* Green background */
  border: 1px solid green; /* Green border */
  color: white; /* White text */
  padding: 10px 24px; /* Some padding */
  cursor: pointer; /* Pointer/hand icon */
  float: center; /* Float the buttons side by side */
}
  </style>
<title>ESP Web Server</title>
<meta name="viewport" content="width=device-width, initial-scale=1">
<link rel="icon" href="data:,">
</head>
<body>
  <div class="topnav">
    <h1>Cheese Press Controller </h1>
  </div>
  <div>
  <div class="content">
    <div class="w3-row">
      <div align="center">
        <label id="currentPressure"  style="text-align:center;font-size:50px;">Current Pressure</label><br/>
        <label id="psi"  style="text-align:center;font-size:90px;">0</label>
        <label id="psiLabel" style="text-align:center;font-size:90px;"> lbs.</label> 
        <p><p>
        <p><p>
          <label id="targetPressure"  style="text-align:center;font-size:50px;">Target Pressure</label><br/>
          <label id="target"  style="text-align:center;font-size:90px;">40</label>
          <br/><!-- comment -->
        <div class="slidecontainer">
          <input type="range" min="10" max="50" value="10" class="slider" id="myRange">
        </div>
        <div class="btn-group">
          
          <button type="submit" class="press" id="press" onclick="press()">PRESS</button>  
          <button type="submit" class="press" id="tare" onclick="tare()">Tare</button>
        </div>
        <div class="btn-group">
            <button type="submit" class="press"  id="release" onclick="release()">RELEASE</button>
          </div>
        <div id="messages"></div>
      </div>
      </div>
      </div>
    </div>
<script>
      var slider = document.getElementById("myRange");
      var target = document.getElementById("target");
      target.innerHTML = slider.value;

      slider.oninput = function () {
          //send('release');
          target.innerHTML = this.value;
      }

      
      
  var gateway = `ws://${window.location.hostname}/ws`;
  var websocket;
  window.addEventListener('load', onLoad);
  function initWebSocket() {
    console.log('Trying to open a WebSocket connection...');
    websocket = new WebSocket(gateway);
    websocket.onopen    = onOpen;
    websocket.onclose   = onClose;
    websocket.onmessage = onMessage; // <-- add this line
  }
  function onOpen(event) {
    console.log('Connection opened');
  }
  function onClose(event) {
    console.log('Connection closed');
    setTimeout(initWebSocket, 2000);
  }
  function onMessage(event) {
    document.getElementById('psi').innerHTML = event.data;
  }
  function onLoad(event) {
    initWebSocket();
    initButton();
  }
  function initButton() {
    document.getElementById('button').addEventListener('click', toggle);
  }
  function toggle(){
    websocket.send('toggle');
  }

  function press() {
    websocket.send(document.getElementById("myRange").value);
  }

  function tare() {
    websocket.send('tare');
  }

  function release() {
    websocket.send('release');
  }



</script>
</body>

)rawliteral";

#endif
