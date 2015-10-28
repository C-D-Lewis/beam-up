var VERSION = "3.0";
var DEBUG = false;

/********************************** Logging ***********************************/

function info(content) {
  console.log(content);
}

function debug(content) {
  if(DEBUG) info(content);
}

/******************************** PebbleKit JS ********************************/

Pebble.addEventListener('showConfiguration', function(e) {
  Pebble.openURL('https://dl.dropboxusercontent.com/u/10824180/pebble%20config%20pages/beamup-basalt-configuration.html?version=' + VERSION);
});

Pebble.addEventListener('webviewclosed', function(e) {
  var json = JSON.parse(decodeURIComponent(e.response));

  var options = {
    'DataKeyDate': '' + json.date,
    'DataKeyAnimations': '' + json.animations,
    'DataKeyBTIndicator': '' + json.bluetooth,
    'DataKeyBatteryMeter': '' + json.battery,
    'DataKeyHourlyVibration': '' + json.hourly
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
