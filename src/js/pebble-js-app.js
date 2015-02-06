var VERSION = "2.3";

Pebble.addEventListener('ready', function(e) {
  console.log('PebbleKit JS ready!');
});

Pebble.addEventListener('showConfiguration', function(e) {
  Pebble.openURL('https://dl.dropboxusercontent.com/u/10824180/pebble%20config%20pages/beamup-configuration.html?version=' + VERSION);
});

Pebble.addEventListener('webviewclosed', function(e) {
  var json = JSON.parse(decodeURIComponent(e.response));

  var options = {
    "PERSIST_KEY_INVERTED": '' + json.inverted,
    "PERSIST_KEY_DATE": '' + json.date,
    "PERSIST_KEY_ANIM": '' + json.animations,
    "PERSIST_KEY_BT": '' + json.bluetooth,
    "PERSIST_KEY_BATTERY": '' + json.battery,
    "PERSIST_KEY_HOURLY": '' + json.hourly,
    "PERSIST_KEY_H_VIBE": +json.hourly_vibe,
    "PERSIST_KEY_BT_VIBE": +json.bluetooth_vibe
  };

  Pebble.sendAppMessage(options,
    function(e) {
      console.log('Settings update successful!');
    },
    function(e) {
      console.log('Settings update failed: ' + JSON.stringify(e));
    });
}); 