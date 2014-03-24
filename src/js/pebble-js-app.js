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
	lights: [],

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

	getSelector: function(index) {
		if (index == 29) return 'all';
		return this.lights[index].id;
	},

	handleResponse: function(xhr, index) {
		appMessageQueue.clear();
		try {
			var res = JSON.parse(xhr.responseText);
			if (res instanceof Array) {
				LIFX.lights = [];
				var i = 0;
				for (var r in res) {
					label = res[r].label ? res[r].label.substring(0,32) : res[r].id.substring(0,32);
					state = res[r].on ? 'ON' : 'OFF';
					color_h = LIFX.colors.hue.toFake(res[r].color.hue) || 0;
					color_s = LIFX.colors.saturation.toFake(res[r].color.saturation) || 0;
					color_b = LIFX.colors.brightness.toFake(res[r].color.brightness) || 0;
					appMessageQueue.add({index:i++, label:label, state:state, color_h:color_h, color_s:color_s, color_b:color_b});
					LIFX.lights.push(res[r]);
				}
				appMessageQueue.add({index:i});
			} else {
				label = res.label ? res.label.substring(0,32) : res.id.substring(0,32);
				state = res.on ? 'ON' : 'OFF';
				color_h = LIFX.colors.hue.toFake(res.color.hue) || 0;
				color_s = LIFX.colors.saturation.toFake(res.color.saturation) || 0;
				color_b = LIFX.colors.brightness.toFake(res.color.brightness) || 0;
				appMessageQueue.add({index:index, label:label, state:state, color_h:color_h, color_s:color_s, color_b:color_b});
				appMessageQueue.add({index:LIFX.lights.length});
			}
		} catch(e) {
			console.log(JSON.stringify(e));
			try {
				var label = LIFX.lights[index].label ? LIFX.lights[index].label.substring(0,32) : LIFX.lights[index].id.substring(0,32);
				appMessageQueue.add({index:index, label:label, state:'Err'});
				appMessageQueue.add({index:LIFX.lights.length});
			} catch(ee) {
				appMessageQueue.add({error:'server_error'});
			}
		}
		appMessageQueue.send();
	},

	color: function(index, hue, saturation, brightness) {
		if (!this.server) return this.error('no_server_set');
		var data = {hue:LIFX.colors.hue.toReal(hue), saturation:LIFX.colors.saturation.toReal(saturation), brightness:LIFX.colors.brightness.toReal(brightness)};
		var url = this.server + '/lights/' + encodeURIComponent(this.getSelector(index)) + '/color.json';
		LIFX.http.makeRequest('PUT', url, JSON.stringify(data), function(xhr) {
			LIFX.handleResponse(xhr, index);
		}, function(e) {
			LIFX.error(e);
		});
	},

	toggle: function(index) {
		if (!this.server) return this.error('no_server_set');
		var url = this.server + '/lights/' + encodeURIComponent(this.getSelector(index)) + '/toggle.json';
		LIFX.http.makeRequest('PUT', url, null, function(xhr) {
			LIFX.handleResponse(xhr, index);
		}, function(e) {
			LIFX.error(e);
		});
	},

	refresh: function() {
		if (!this.server) return this.error('no_server_set');
		this.lights = [];
		var url = this.server + '/lights.json';
		LIFX.http.makeRequest('GET', url, null, function(xhr) {
			LIFX.handleResponse(xhr);
		}, function(e) {
			LIFX.error(e);
		});
	},

	http: {
		makeRequest: function(method, url, data, cb, fb) {
			var xhr = new XMLHttpRequest();
			xhr.open(method, url, true);
			xhr.onload = function() { cb(xhr); };
			xhr.onerror = function() { fb('error'); };
			xhr.ontimeout = function() { fb('timeout'); };
			xhr.timeout = 10000;
			xhr.send(data);
		}
	}
};

Pebble.addEventListener('ready', function(e) {
	LIFX.refresh();
});

Pebble.addEventListener('appmessage', function(e) {
	console.log('AppMessage received: ' + JSON.stringify(e.payload));
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
