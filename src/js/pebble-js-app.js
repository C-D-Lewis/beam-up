var VERSION = "3.1";
var DEBUG = false;

/*************************** Weather library start ****************************/

var owmWeatherAPIKey = '';

function owmWeatherXHR(url, type, callback) {
  var xhr = new XMLHttpRequest();
  xhr.onload = function () {
    callback(this.responseText);
  };
  xhr.open(type, url);
  xhr.send();
}

function owmWeatherSendToPebble(json) {
  Pebble.sendAppMessage({
    'OWMWeatherAppMessageKeyReply': 1,
    'OWMWeatherAppMessageKeyDescription': json.weather[0].description,
    'OWMWeatherAppMessageKeyDescriptionShort': json.weather[0].main,
    'OWMWeatherAppMessageKeyTempK': json.main.temp,
    'OWMWeatherAppMessageKeyName': json.name
  });
}

function owmWeatherLocationSuccess(pos) {
  var url = 'http://api.openweathermap.org/data/2.5/weather?lat=' +
    pos.coords.latitude + '&lon=' + pos.coords.longitude + '&appid=' + owmWeatherAPIKey;
  console.log('owm-weather: Location success. Contacting OpenWeatherMap.org...');

  owmWeatherXHR(url, 'GET', function(responseText) {
    console.log('owm-weather: Got API response!');
    if(responseText.length > 100) {
      owmWeatherSendToPebble(JSON.parse(responseText));
    } else {
      console.log('owm-weather: API response was bad. Wrong API key?');
      Pebble.sendAppMessage({
        'OWMWeatherAppMessageKeyBadKey': 1
      });
    }
  });
}

function owmWeatherLocationError(err) {
  console.log('owm-weather: Location error');
  Pebble.sendAppMessage({
    'OWMWeatherAppMessageKeyLocationUnavailable': 1
  });
}

function owmWeatherHandler(dict) {
  if(dict.payload['OWMWeatherAppMessageKeyRequest']) {
    owmWeatherAPIKey = dict.payload['OWMWeatherAppMessageKeyRequest'];
    console.log('owm-weather: Got fetch request from C app');

    navigator.geolocation.getCurrentPosition(owmWeatherLocationSuccess, owmWeatherLocationError, {
      timeout: 15000,
      maximumAge: 60000
    });
  }
};

/**************************** Weather library end *****************************/

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

  owmWeatherHandler(dict);
});
