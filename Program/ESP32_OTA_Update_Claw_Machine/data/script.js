// Blessed https://randomnerdtutorials.com/esp32-ota-over-the-air-arduino/
// https://randomnerdtutorials.com/esp32-websocket-server-arduino/
// https://yoannmoi.net/nipplejs/
// Sankiu https://moshfeu.medium.com/how-to-build-an-html5-games-controller-with-arduino-nodejs-and-socket-io-part-2-bbd01bf36481
// JSON https://m1cr0lab-esp32.github.io/remote-control-with-websocket/websocket-and-json/

// ----------------------------------------------------------------------------
// Definition of global variables
// ----------------------------------------------------------------------------
var gateway = `ws://${window.location.hostname}/ws`;
var websocket;

//Create deadzones for main cardinal directions.
var deadzoneAngle = 25.0;

var angleRight1 = 0.0,
    angleUp = 90.0,
    angleLeft = 180.0,
    angleDown = 270.0,
    angleRight2 = 360.0;

//Remember last sent directions, only update on change.
var leftLastDirY, rightLastDirY, rightLastDirX;

//Call onLoad when the client loads the page.
window.addEventListener('load', onLoad);

function onLoad(event) {
    initWebSocket();
    createAndAssignJoysticks();
}

function initWebSocket() {
    console.log('Trying to open a WebSocket connection...');
    websocket = new WebSocket(gateway);
    websocket.onopen = onOpen;
    websocket.onclose = onClose;
    websocket.onmessage = onMessage;
}

function onOpen(event) {
    obj = {
        "id": "gpiostates"
    };

    console.log('Connection opened');
    sendJSONData(obj); //Asking server for GPIO States to ESP32
}

function onClose(event) {
    console.log('Connection closed');
    setTimeout(initWebSocket, 2000);
}

//Receives an JSON variable from ESP32 and puts the data into the HTML
function onMessage(event) {
    console.log("------------------------------");
    var myObj = JSON.parse(event.data);
    if (myObj.hasOwnProperty("id")) {
        //console.log(myObj);
        //Create actions for the JSON OBJS
        switch (myObj.id) {
            case "gpiostates":
                console.log("gpiostates");
                gpioStatesReceived(myObj);
                break;
            case "pin":
                console.log("pin");
                pinReceived(myObj);
                break;
            case "joystick":
                console.log("joystick");
                joystickReceived(myObj);
                break;
        }
    }
    //console.log("------------------------------");
}

function gpioStatesReceived(jsonData) {
    for (i in jsonData.gpios) {
        var output = jsonData.gpios[i].output;
        var state = jsonData.gpios[i].state;
        if (state == "1") {
            document.getElementById(output).checked = true;
            document.getElementById(output + "s").innerHTML = "ON";
        }
        else {
            document.getElementById(output).checked = false;
            document.getElementById(output + "s").innerHTML = "OFF";
        }
    }
}
function pinReceived(jsonData) {
    console.log("Pin received:");
    console.log(jsonData);
}

function joystickReceived(jsonData) {
    console.log("Joystick received:");
    console.log(jsonData);
}

function sendJSONData(data) {
    websocket.send(JSON.stringify(data));
}

function createAndAssignJoysticks() {
    createAndAssignLeftJoystick();
    createAndAssignRightJoystick();
}

//Left joystick claw Z axis "lockY" init.
//Left joystick events
function createAndAssignLeftJoystick() {
    var joystickL = nipplejs.create({
        zone: document.getElementById('jLeft'),
        mode: 'dynamic',
        color: 'green',
        lockY: true,
        threshold: 0.5,
        size: 50
    });

    joystickL
        .on('added', function (evt, nipple) {
            nipple.on('start dir end', function (evt, data) {
                var directionY = handleLeftJoystickEvents(evt, data);
                if (leftLastDirY != directionY) {
                    passJoystickDirection("bi", evt, directionY);
                    leftLastDirY = directionY;
                }
            });
        })
        .on('removed', function (evt, nipple) {
            nipple.off('start move end dir plain');
        });
}

//if the event is triggered by the direction change, then log direction.
//Otherwise (start/end) direction resets to "stop".
function handleLeftJoystickEvents(evt, dirData) {
    if (evt.type == "dir") {
        return dirData.direction?.y;
    }
    return "stop";
}

//Left joystick claw X and Y axis init.
//Right joystick events.
function createAndAssignRightJoystick() {
    var joystickR = nipplejs.create({
        zone: document.getElementById('jRight'),
        mode: 'dynamic',
        color: 'red',
        threshold: 0.5,
        size: 50
    });
    joystickR
        .on('added', function (evt, nipple) {
            nipple.on('start move end', function (evt, data) {
                var [directionY, directionX] = handleRightJoystickEvents(evt, data);
                if (rightLastDirY != directionY || rightLastDirX != directionX) {
                    passJoystickDirection("quad", evt, directionY, directionX);
                    rightLastDirY = directionY;
                    rightLastDirX = directionX;
                }
            });
        })
        .on('removed', function (evt, nipple) {
            nipple.off('start move end dir plain');
        });
}

//Check movement composition.
//If angle X is 0<X<180 then direction up, otherwise down, same for left/right.
//Adjust a deadzone so that when joystick in any main cardinal position It performs that one.
//Example: if angle is East+-deadzoneAngle then thats Right.Otherwise would be RigtUp or RightDown.
//0/360(Right) or 180(Left): Stop Y axis
function handleYAxis(moveData) {
    if (((angleRight2 - deadzoneAngle) < moveData.angle.degree || moveData.angle.degree < (angleRight1 + deadzoneAngle)) ||
        ((angleLeft - deadzoneAngle) < moveData.angle.degree && moveData.angle.degree < (angleLeft + deadzoneAngle))) {
        return "stop";
    }
    if (angleRight1 <= moveData.angle.degree && moveData.angle.degree <= angleLeft) {
        return "up";
    }
    return "down";
}

//90(Up) or 270(Down): Stop X axis
function handleXAxis(moveData) {
    if ((angleUp - deadzoneAngle) < moveData.angle.degree && moveData.angle.degree < (angleUp + deadzoneAngle) ||
        (angleDown - deadzoneAngle) < moveData.angle.degree && moveData.angle.degree < (angleDown + deadzoneAngle)) {
        return "stop";
    }
    if (angleUp < moveData.angle.degree && moveData.angle.degree < angleDown) {
        return "left";
    }
    return "right";
}

function handleRightJoystickEvents(evt, data) {
    if (evt.type == "move") {
        return [handleYAxis(data), handleXAxis(data)];
    }
    return ["stop", "stop"];
}

//Send Requests to toggle GPIOs.
//Act on html.
function toggleCheckbox(element) {
    obj = {
        "id": "pin",
        "action": "toggle",
        "number": element.id
    };
    if (element.checked) {
        document.getElementById(element.id + "s").innerHTML = "ON";
    }
    else {
        document.getElementById(element.id + "s").innerHTML = "OFF";
    }
    console.log(obj.action + ": " + obj.number + " | ID: " + obj.id);

    sendJSONData(obj);
}

//Send joystick directions as json object. 
function passJoystickDirection(joystick, evt, directionY = "stop", directionX = "stop") {
    obj = {
        "id": "joystick",
        "controller": joystick,
        "directionY": directionY,
        "directionX": directionX
    };
    console.log(joystick + " joystick: Y=" + directionY + ", X=" + directionX + ", EVENT TRIGGER: " + evt.type);

    sendJSONData(obj);
}