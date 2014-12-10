var VERSION = "2.3";

Pebble.addEventListener('ready', function(e) {
  console.log('PebbleKit JS ready!');
});

Pebble.addEventListener('showConfiguration', function(e) {
  Pebble.openURL('https://dl.dropboxusercontent.com/u/10824180/pebble%20config%20pages/beamup-configuration.html?version=' + VERSION);
});

Pebble.addEventListener('webviewclosed', function(e) {
  console.log(JSON.stringify(e.response));
}); 