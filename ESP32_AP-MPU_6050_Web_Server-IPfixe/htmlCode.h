const char *HTML_CONTENT = R""""(

<!DOCTYPE HTML><html>
<head>
  <title>MFE Synth - ESP Web Server</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="icon" href="data:,">

 <style> 
html {
  font-family: Arial;
  display: inline-block;
  text-align: center;
}
p {
  font-size: 1.2rem;
}
body {
  margin: 0;
}
.topnav {
  overflow: hidden;
  background-color: #003366;
  color: #FFD43B;
  font-size: 1rem;
}
.content {
  padding: 20px;
}
.card {
  background-color: white;
  box-shadow: 2px 2px 12px 1px rgba(140,140,140,.5);
}
.card-title {
  color:#003366;
  font-weight: bold;
}
.cards {
  max-width: 800px;
  margin: 0 auto;
  display: grid; grid-gap: 2rem;
  grid-template-columns: repeat(auto-fit, minmax(200px, 1fr));
}
.reading {
  font-size: 1.2rem;
}
.cube-content{
  width: 100%;
  background-color: white;
  height: 300px; margin: auto;
  padding-top:2%;
}
#reset{
  border: none;
  color: #FEFCFB;
  background-color: #003366;
  padding: 10px;
  text-align: center;
  display: inline-block;
  font-size: 14px; width: 150px;
  border-radius: 4px;
}
#resetX, #resetY, #resetZ{
  border: none;
  color: #FEFCFB;
  background-color: #003366;
  padding-top: 10px;
  padding-bottom: 10px;
  text-align: center;
  display: inline-block;
  font-size: 14px;
  width: 20px;
  border-radius: 4px;
}
 </style>
</head>

<body>
  <div class="topnav">
    <h1><i class="far fa-compass"></i> MFE <i class="far fa-compass"></i></h1>
  </div>
  <div class="content">
    <div class="cards">
      <div class="card">
        <p class="card-title">GYROSCOPE</p>
        <p><span class="reading">X: <span id="gyroX"></span> rad</span></p>
        <p><span class="reading">Y: <span id="gyroY"></span> rad</span></p>
        <p><span class="reading">Z: <span id="gyroZ"></span> rad</span></p>
      </div>
      <div class="card">
        <p class="card-title">ACCELEROMETER</p>
        <p><span class="reading">X: <span id="accX"></span> ms<sup>2</sup></span></p>
        <p><span class="reading">Y: <span id="accY"></span> ms<sup>2</sup></span></p>
        <p><span class="reading">Z: <span id="accZ"></span> ms<sup>2</sup></span></p>
      </div>
      <div class="card">
        <p class="card-title">TEMPERATURE</p>
        <p><span class="reading"><span id="temp"></span> &deg;C</span></p>
      </div>
    </div>
  <p><button id="start">Start</button></p>
  <p><button id="stop">Stop</button></p>
  </div>

<script>

const AudioContext = window.AudioContext || window.webkitAudioContext;
let audioContext;

let mod = null; 
let carrier = null; 
//let initialized = false;

const defaultModFreq = 30;
const defaultCarrierFreq = 300;

const $start = document.querySelector('#start');
const $stop = document.querySelector('#stop');
const $carrier = document.querySelector('#gyroX');
const $mod = document.querySelector('#gyroY');

$carrier.value = defaultCarrierFreq;
$mod.value = defaultModFreq;

$stop.setAttribute("disabled", "disabled");

$start.onclick = () => {

    $start.setAttribute("disabled", "disabled");
    $stop.removeAttribute("disabled");

    audioContext = new AudioContext();
  
    // create AM synth (or probably ring modulation...)
    const tremolo = audioContext.createGain();
    tremolo.gain.value = 0.5;
    tremolo.connect(audioContext.destination);

    const depth = audioContext.createGain();
    depth.gain.value = 0.5;
    depth.connect(tremolo.gain);

    mod = audioContext.createOscillator();
    mod.frequency.value = defaultModFreq;
    mod.connect(depth);

    carrier = audioContext.createOscillator();
    carrier.frequency.value = defaultCarrierFreq;
    carrier.connect(tremolo);

    carrier.start();
    mod.start();

    audioContext.onstatechange = function () {
    console.log(audioContext.state);
    };
  
};

// Close the audiocontext
$stop.onclick = () => {
 audioContext.close().then(() => {
  $start.removeAttribute("disabled");
  // Reset the text of the suspend/resume toggle:
  $stop.setAttribute("disabled", "disabled");
  });
};

function updateModFreq() {
  if (mod !== null) {
    mod.frequency.value = this.value;
  }
}

function updateCarrierFreq() {
  if (carrier !== null) {
    carrier.frequency.value = this.value;
  }
}


$mod.addEventListener('input', updateModFreq);
$carrier.addEventListener('input', updateCarrierFreq);




/*
Catch controls values exemple
//node
    audioDry = audioContext.createGain();

//Defaut Value
    audioDry.gain.value = 0.96;

audioDry = document.querySelector('#audioDry');
audioDry.addEventListener('input', function() {
    audioDry.gain.value = this.value;
    }, false);
*/


// Create events for the sensor readings
if (!!window.EventSource) {
  var source = new EventSource('/events');

  source.addEventListener('open', function(e) {
    console.log("Events Connected");
  }, false);

  source.addEventListener('error', function(e) {
    if (e.target.readyState != EventSource.OPEN) {
      console.log("Events Disconnected");
    }
  }, false);

  source.addEventListener('gyro_readings', function(e) {
    //console.log("gyro_readings", e.data);
    var obj = JSON.parse(e.data);
    document.getElementById("gyroX").innerHTML = obj.gyroX;
    document.getElementById("gyroY").innerHTML = obj.gyroY;
    document.getElementById("gyroZ").innerHTML = obj.gyroZ;

  
    carrier.frequency.value = obj.gyroX * 200;
    mod.frequency.value = obj.gyroY * 20;

    /* Change cube rotation after receiving the readinds
    cube.rotation.x = obj.gyroY;
    cube.rotation.z = obj.gyroX;
    cube.rotation.y = obj.gyroZ;
    renderer.render(scene, camera); */
  }, false);
   

  source.addEventListener('temperature_reading', function(e) {
    console.log("temperature_reading", e.data);
    document.getElementById("temp").innerHTML = e.data;
  }, false);

  source.addEventListener('accelerometer_readings', function(e) {
    console.log("accelerometer_readings", e.data);
    var obj = JSON.parse(e.data);
    var keys = Object.keys(obj);
    for (var i = 0; i < keys.length; i++){
        var key = keys[i];
        document.getElementById(key).innerHTML = obj[key];
        }
   // document.getElementById("accX").innerHTML = obj.accX;
   // document.getElementById("accY").innerHTML = obj.accY;
   // document.getElementById("accZ").innerHTML = obj.accZ;

/*
        var myObj = JSON.parse(event.data);
        var keys = Object.keys(myObj);

    for (var i = 0; i < keys.length; i++){
        var key = keys[i];

        document.getElementById(key).innerHTML = myObj[key];
        }
*/
  }, false);
}


</script>
</body>

</html>
)"""";