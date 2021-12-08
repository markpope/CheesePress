#ifndef _CONSOLE_H
#define _CONSOLE_H

const char console_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <title>Cheese Press Console </title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="icon" href="data:,">
</head>
<body>
  
<script>
   
      
  var gateway = `ws://${window.location.hostname}/console`;
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
    document.getElementById('output').innerHTML += event.data + '<br>';
    document.getElementById( 'bottom' ).scrollIntoView();
  }
  function onLoad(event) {
    initWebSocket();
  }

</script>

<code>
  <div id="output">...</div>
  <div id="bottom">...</div>
</code>

</body>
</html>
)rawliteral";

#endif
