var VERSION = "2.4";

/******************************** Pebble API **********************************/

Pebble.addEventListener('ready', function(e) {
  console.log('PebbleKit JS ready!');
});

Pebble.addEventListener('showConfiguration', function(e) {
  Pebble.openURL('https://dl.dropboxusercontent.com/u/10824180/pebble%20config%20pages/beamup-basalt-configuration.html?version=' + VERSION);
});

Pebble.addEventListener('webviewclosed', function(e) {
  var json = JSON.parse(decodeURIComponent(e.response));

  var options = {
    'PERSIST_KEY_DATE': '' + json.date,
    'PERSIST_KEY_ANIM': '' + json.animations,
    'PERSIST_KEY_BT': '' + json.bluetooth,
    'PERSIST_KEY_BATTERY': '' + json.battery,
    'PERSIST_KEY_HOURLY': '' + json.hourly,
    'PERSIST_KEY_THEME': '' + json.theme
  };

  Pebble.sendAppMessage(options,
    function(e) {
      console.log('Settings update successful!');
    },
    function(e) {
      console.log('Settings update failed: ' + JSON.stringify(e));
    });
}); 