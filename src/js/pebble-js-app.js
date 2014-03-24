var appMessageQueue = {
	queue: [],
	numTries: 0,
	maxTries: 5,
	add: function(obj) {
		this.queue.push(obj);
	},
	clear: function() {
		this.queue = [];
	},
	isEmpty: function() {
		return this.queue.length === 0;
	},
	nextMessage: function() {
		return this.isEmpty() ? {} : this.queue[0];
	},
	send: function() {
		if (this.queue.length > 0) {
			var ack = function() {
				appMessageQueue.numTries = 0;
				appMessageQueue.queue.shift();
				appMessageQueue.send();
			};
			var nack = function() {
				appMessageQueue.numTries++;
				appMessageQueue.send();
			};
			if (this.numTries >= this.maxTries) {
				console.log('Failed sending AppMessage: ' + JSON.stringify(this.nextMessage()));
				ack();
			}
			console.log('Sending AppMessage: ' + JSON.stringify(this.nextMessage()));
			Pebble.sendAppMessage(this.nextMessage(), ack, nack);
		}
	}
};

var LIFX = {
	server: localStorage.getItem('server') || '',
	devices: [],
	colors: {
		hue: {
			toFake: function(val) {
				return parseInt((val / 360) * 100);
			},
			toReal: function(val) {
				return val * 3.6;
			}
		},
		saturation: {
			toFake: function(val) {
				return parseInt(val * 100);
			},
			toReal: function(val) {
				return val / 100;
			}
		},
		brightness: {
			toFake: function(val) {
				return parseInt(val * 100);
			},
			toReal: function(val) {
				return val / 100;
			}
		}
	},
	error: function(error) {
		appMessageQueue.clear();
		appMessageQueue.add({error:error});
		appMessageQueue.send();
	},
	color: function(index, hue, saturation, brightness) {
		if (!this.server) return this.error('no_server_set');
		var post_data = {hue:LIFX.colors.hue.toReal(hue), saturation:LIFX.colors.saturation.toReal(saturation), brightness:LIFX.colors.brightness.toReal(brightness)};
		var xhr = new XMLHttpRequest();
		xhr.open('PUT', this.server + '/lights/' + encodeURIComponent(this.devices[index].id) + '/color.json', true);
		xhr.onload = function() {
			appMessageQueue.clear();
			try {
				var res = JSON.parse(xhr.responseText);
				var label = res.label ? res.label.substring(0,32) : res.id.substring(0,32);
				var state = res.on ? 'ON' : 'OFF';
				var color_h = LIFX.colors.hue.toFake(res.color.hue) || 0;
				var color_s = LIFX.colors.saturation.toFake(res.color.saturation) || 0;
				var color_b = LIFX.colors.brightness.toFake(res.color.brightness) || 0;
				appMessageQueue.add({index:index, label:label, state:state, color_h:color_h, color_s:color_s, color_b:color_b});
				appMessageQueue.add({index:LIFX.devices.length});
			} catch(e) {
				appMessageQueue.add({index:index, label:LIFX.devices[index].label, state:'Err'});
				appMessageQueue.add({index:LIFX.devices.length});
			}
			appMessageQueue.send();
		};
		xhr.ontimeout = function() { LIFX.error('timeout'); };
		xhr.onerror = function() { LIFX.error('error'); };
		xhr.timeout = 10000;
		xhr.send(post_data);
	},
	toggle: function(index) {
		if (!this.server) return this.error('no_server_set');
		var xhr = new XMLHttpRequest();
		xhr.open('PUT', this.server + '/lights/' + encodeURIComponent(this.devices[index].id) + '/toggle.json', true);
		xhr.onload = function() {
			appMessageQueue.clear();
			try {
				var res = JSON.parse(xhr.responseText);
				var label = res.label ? res.label.substring(0,32) : res.id.substring(0,32);
				var state = res.on ? 'ON' : 'OFF';
				var color_h = LIFX.colors.hue.toFake(res.color.hue) || 0;
				var color_s = LIFX.colors.saturation.toFake(res.color.saturation) || 0;
				var color_b = LIFX.colors.brightness.toFake(res.color.brightness) || 0;
				appMessageQueue.add({index:index, label:label, state:state, color_h:color_h, color_s:color_s, color_b:color_b});
				appMessageQueue.add({index:LIFX.devices.length});
			} catch(e) {
				appMessageQueue.add({index:index, label:LIFX.devices[index].label, state:'Err'});
				appMessageQueue.add({index:LIFX.devices.length});
			}
			appMessageQueue.send();
		};
		xhr.ontimeout = function() { LIFX.error('timeout'); };
		xhr.onerror = function() { LIFX.error('error'); };
		xhr.timeout = 10000;
		xhr.send(null);
	},
	refresh: function() {
		if (!this.server) return this.error('no_server_set');
		this.devices = [];
		var xhr = new XMLHttpRequest();
		xhr.open('GET', this.server + '/lights.json', true);
		xhr.onload = function() {
			appMessageQueue.clear();
			try {
				var res = JSON.parse(xhr.responseText);
				var i = 0;
				for (var r in res) {
					var label = res[r].label ? res[r].label.substring(0,32) : res[r].id.substring(0,32);
					var state = res[r].on ? 'ON' : 'OFF';
					var color_h = LIFX.colors.hue.toFake(res[r].color.hue) || 0;
					var color_s = LIFX.colors.saturation.toFake(res[r].color.saturation) || 0;
					var color_b = LIFX.colors.brightness.toFake(res[r].color.brightness) || 0;
					appMessageQueue.add({index:i++, label:label, state:state, color_h:color_h, color_s:color_s, color_b:color_b});
					LIFX.devices.push(res[r]);
				}
				appMessageQueue.add({index:i});
			} catch(e) {
				appMessageQueue.add({error:'server_error'});
			}
			appMessageQueue.send();
		};
		xhr.ontimeout = function() { LIFX.error('timeout'); };
		xhr.onerror = function() { LIFX.error('error'); };
		xhr.timeout = 10000;
		xhr.send(null);
	}
};

Pebble.addEventListener('ready', function(e) {
	LIFX.refresh();
});

Pebble.addEventListener('appmessage', function(e) {
	if (isset(e.payload.index)) {
		if (isset(e.payload.color_h) && isset(e.payload.color_s) && isset(e.payload.color_b)) {
			LIFX.color(e.payload.index, e.payload.color_h, e.payload.color_s, e.payload.color_b);
		} else {
			LIFX.toggle(e.payload.index);
		}
	} else {
		LIFX.refresh();
	}
});

Pebble.addEventListener('showConfiguration', function() {
	var uri = 'https://rawgithub.com/Neal/PebbLIFX/master/configuration/index.html?server=' + encodeURIComponent(LIFX.server);
	Pebble.openURL(uri);
});

Pebble.addEventListener('webviewclosed', function(e) {
	if (e.response) {
		var data = JSON.parse(decodeURIComponent(e.response));
		if (data.server) {
			LIFX.server = data.server;
			localStorage.setItem('server', LIFX.server);
			LIFX.refresh();
		}
	}
});

function isset(i) {
	return (typeof i != 'undefined');
}
