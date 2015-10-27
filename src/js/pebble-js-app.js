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

Pebble.addEventListener('ready', function() {
  info('PebbleKit JS ready! Version ' + VERSION);
});

Pebble.addEventListener('appmessage', function(dict) {
  debug('Got appmessage: ' + JSON.stringify(dict.payload));
});
