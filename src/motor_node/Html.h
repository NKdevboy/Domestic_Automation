#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>



//--------------------------------------------------
// HTML PAGE
//--------------------------------------------------

const char webpage[] PROGMEM = R"=====(

<!DOCTYPE html>
<html>

<head>

<meta name="viewport"
content="width=device-width, initial-scale=1.0">

<title>IoT Dashboard</title>

<style>

*{
    box-sizing:border-box;
}

body{

    margin:0;

    padding:6px;

    font-family:Arial;

    background:#f2f2f2;

    min-height:100vh;

    overflow-y:auto;

    overflow-x:hidden;
}

.container{

    display:flex;

    flex-direction:column;

    gap:6px;
}

.card{

    background:white;

    border:2px solid #444;

    border-radius:16px;

    padding:7px;
}

.title{

    text-align:center;

    font-size:18px;

    font-weight:bold;

    margin-bottom:6px;
}

.row{

    display:flex;

    justify-content:space-between;

    align-items:center;

    font-size:16px;

    margin:3px 0;
}

.centerText{

    text-align:center;

    font-size:18px;

    font-weight:bold;
}

.btnRow{

    display:flex;

    justify-content:space-around;

    margin-top:6px;
}

button{

    width:42%;

    padding:8px;

    border:none;

    border-radius:10px;

    font-size:16px;

    color:black;
}

.startBtn{

    background:#b8e6b8;

    border:1px solid green;
}

.stopBtn{

    background:#f3c1c1;

    border:1px solid red;
}

.autoInput{

    width:120px;

    padding:6px;

    font-size:16px;
}

.autoRow{

    margin-top:10px;

    text-align:center;

    font-size:16px;
}

.smallText{

    font-size:16px;
}

</style>

</head>

<body>

<div class="container">

<!-- TOP STATUS -->

<div class="card">

<div class="row">

<div>
Motor Alive Cnt :
<span id="s1Alive">--</span>
</div>

<div>
Tank Alive Cnt :
<span id="s2Alive">--</span>
</div>

</div>

</div>

<!-- SUPPLY -->

<div class="card">

<div class="title">
Supply Status
</div>

<div class="row">

<div>
Voltage :
<span id="voltage">--</span> V
</div>

<div>
Current :
<span id="current">--</span> A
</div>

</div>

</div>

<!-- BORE MOTOR -->

<div class="card">

<div class="centerText">

Bore Motor Status :
<span id="motorStatus">--</span>

</div>

<div class="btnRow">

<button class="startBtn"
onclick="motorStart()">
START
</button>

<button class="stopBtn"
onclick="motorStop()">
STOP
</button>

</div>

<!-- AUTO OFF -->

<div class="autoRow">

Auto OFF Time :

<input type="number"
id="autoOffTime"
class="autoInput"
placeholder="Minutes"
value="21">

mins

</div>

<div class="btnRow">

<button class="startBtn"
onclick="enableAutoOff()">
Auto Enable
</button>

<button class="stopBtn"
onclick="disableAutoOff()">
Auto Disable
</button>

</div>

<div class="autoRow">

Auto OFF :
<span id="autoStatus">DISABLED</span>

</div>

<div class="autoRow">

Remaining :
<span id="remainingTime">-- m -- s</span>

</div>

</div>

<!-- VALVE -->

<div class="card">

<div class="centerText">

Mettur Valve Status :
<span id="valveStatus">--</span>

</div>

<div class="btnRow">

<button class="startBtn"
onclick="valveOpen()">
OPEN
</button>

<button class="stopBtn"
onclick="valveClose()">
CLOSE
</button>

</div>

</div>

<!-- BORE TANK -->

<div class="card">

<div class="title">
Bore Tank
</div>

<div class="row smallText">

<div>
Percentage :
<span id="boreTank">--</span> %
</div>

<div>
Empty Height :
<span id="boreEmpty">--</span> Ft
</div>

</div>

</div>

<!-- METTUR TANK -->

<div class="card">

<div class="title">
Mettur Tank
</div>

<div class="row smallText">

<div>
Percentage :
<span id="metturTank">--</span> %
</div>

<div>
Empty Height :
<span id="metturEmpty">--</span> Ft
</div>

</div>

</div>

</div>

<script>

async function updateData(){

    try{

        const response =
            await fetch('/getData');

        const data =
            await response.json();

        document.getElementById("s1Alive").innerHTML =
            data.s1Alive;

        document.getElementById("s2Alive").innerHTML =
            data.s2Alive;

        document.getElementById("voltage").innerHTML =
            data.voltage;

        document.getElementById("current").innerHTML =
            data.current;

        document.getElementById("motorStatus").innerHTML =
            data.motorStatus;

        document.getElementById("valveStatus").innerHTML =
            data.valveStatus;

        document.getElementById("boreTank").innerHTML =
            data.boreTank;

        document.getElementById("boreEmpty").innerHTML =
            data.boreEmpty;

        document.getElementById("metturTank").innerHTML =
            data.metturTank;

        document.getElementById("metturEmpty").innerHTML =
            data.metturEmpty;

        document.getElementById("autoStatus").innerHTML =
            data.autoStatus;
        
        document.getElementById("remainingTime").innerHTML =
            data.remainingTime;
    }
    catch(err){

        console.log(err);
    }
}

async function motorStart(){

    await fetch('/bore/start');
}

async function motorStop(){

    await fetch('/bore/stop');
}

async function valveOpen(){

    await fetch('/valve/open');
}

async function valveClose(){

    await fetch('/valve/close');
}

async function enableAutoOff(){

    let mins = document.getElementById("autoOffTime").value;

    await fetch('/autooff/enable?mins=' + mins);

}


async function disableAutoOff(){

    await fetch('/autooff/disable');
}

setInterval(updateData,2000);

updateData();

</script>

</body>
</html>

)=====";