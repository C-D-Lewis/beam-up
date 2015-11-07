var VERSION = "3.0";
var DEBUG = false;

// https://github.com/smallstoneapps/gcolor.js/blob/master/lib/gcolor.js
function GColorFromHex(hex) {
  var hexNum = parseInt(hex, 16);
  var a = 192;
  var r = (((hexNum >> 16) & 0xFF) >> 6) << 4;
  var g = (((hexNum >>  8) & 0xFF) >> 6) << 2;
  var b = (((hexNum >>  0) & 0xFF) >> 6) << 0;
  return a + r + g + b;
}

/********************************** Logging ***********************************/

function info(content) {
  console.log(content);
}

function debug(content) {
  if(DEBUG) info(content);
}

/******************************** PebbleKit JS ********************************/

Pebble.addEventListener('showConfiguration', function(e) {
  Pebble.openURL('https://dl.dropboxusercontent.com/u/10824180/pebble%20config%20pages/beamup-basalt-configuration-testing.html?version=' + VERSION);
});

Pebble.addEventListener('webviewclosed', function(e) {
  var json = JSON.parse(decodeURIComponent(e.response));

  // Convert colors
  var foreground = GColorFromHex(json.foreground);
  var background = GColorFromHex(json.background);

  var options = {
    'DataKeyDate': '' + json.date,
    'DataKeyAnimations': '' + json.animations,
    'DataKeyBTIndicator': '' + json.bluetooth,
    'DataKeyHourlyVibration': '' + json.hourly,
    'DataKeyForegroundColor': foreground,
    'DataKeyBackgroundColor': background
  };

  Pebble.sendAppMessage(options, function() {
    console.log('Settings update successful!');
  }, function(e) {
    console.log('Settings update failed: ' + JSON.stringify(e));
  });
});

Pebble.addEventListener('ready', function() {
  info('PebbleKit JS ready! Version ' + VERSION);
});

Pebble.addEventListener('appmessage', function(dict) {
  debug('Got appmessage: ' + JSON.stringify(dict.payload));
});
