var appMessageQueue = {
	queue: [],
	numTries: 0,
	maxTries: 5,
	working: false,
	clear: function() {
		this.queue = [];
		this.working = false;
	},
	isEmpty: function() {
		return this.queue.length === 0;
	},
	nextMessage: function() {
		return this.isEmpty() ? {} : this.queue[0];
	},
	send: function(message) {
		if (message) this.queue.push(message);
		if (this.working) return;
		if (this.queue.length > 0) {
			this.working = true;
			var ack = function() {
				appMessageQueue.numTries = 0;
				appMessageQueue.queue.shift();
				appMessageQueue.working = false;
				appMessageQueue.send();
			};
			var nack = function() {
				appMessageQueue.numTries++;
				appMessageQueue.working = false;
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

var TYPE = {
	ERROR: 0,
	LIGHT: 1,
	TAG: 2,
	ALL: 3
};

var METHOD = {
	BEGIN: 0,
	DATA: 1,
	END: 2,
	REFRESH: 3,
	TOGGLE: 4,
	COLOR: 5,
	READY: 6
};

var LIFX = {
	server: localStorage.getItem('server') || 'http://lifx-http.local:56780',
	lights: [],
	tags: [],
	type: null,
	method: null,
	index: 0,

	colors: {
		makePostData: function(hue, saturation, brightness, kelvin) {
			return {hue:this.hue.deserialize(hue), saturation:this.saturation.deserialize(saturation), brightness:this.brightness.deserialize(brightness), kelvin:this.kelvin.deserialize(kelvin)};
		},
		hue: {
			serialize: function(val) { return Math.round((val / 360) * 100); },
			deserialize: function(val) { return val * 3.6; }
		},
		saturation: {
			serialize: function(val) { return Math.round(val * 100); },
			deserialize: function(val) { return val / 100; }
		},
		brightness: {
			serialize: function(val) { return Math.round(val * 100); },
			deserialize: function(val) { return val / 100; }
		},
		kelvin: {
			serialize: function(val) { return val; },
			deserialize: function(val) { return val; }
		}
	},

	error: function(error) {
		appMessageQueue.clear();
		appMessageQueue.send({type:TYPE.ERROR, label:error});
	},

	getSelector: function() {
		switch (this.type) {
			default:
			case TYPE.ALL:
				return 'all';
			case TYPE.LIGHT:
				return this.lights[this.index].id;
			case TYPE.TAG:
				return 'tag:' + this.tags[this.index].label;
		}
	},

	sendLight: function(index) {
		var label = this.lights[index].label ? this.lights[index].label.substring(0,18) : this.lights[index].id;
		var state = this.lights[index].on ? 'ON' : 'OFF';
		var color_h = 50, color_s = 100, color_b = 100, color_k = 3000;
		if (this.lights[index].color) {
			color_h = LIFX.colors.hue.serialize(this.lights[index].color.hue);
			color_s = LIFX.colors.saturation.serialize(this.lights[index].color.saturation);
			color_b = LIFX.colors.brightness.serialize(this.lights[index].color.brightness);
			color_k = LIFX.colors.kelvin.serialize(this.lights[index].color.kelvin);
		}
		appMessageQueue.send({type:TYPE.LIGHT, method:METHOD.DATA, index:index, label:label, state:state, color_h:color_h, color_s:color_s, color_b:color_b, color_k:color_k});
	},

	sendLights: function() {
		appMessageQueue.send({type:TYPE.LIGHT, method:METHOD.BEGIN, index:LIFX.lights.length});
		for (var i = 0; i < LIFX.lights.length; i++) {
			this.sendLight(i);
		}
		appMessageQueue.send({type:TYPE.LIGHT, method:METHOD.END});
	},

	sendTags: function() {
		appMessageQueue.send({type:TYPE.TAG, method:METHOD.BEGIN, index:LIFX.tags.length});
		for (var i = 0; i < LIFX.tags.length; i++) {
			var label = this.tags[i].label ? this.tags[i].label.substring(0,18) : '';
			var color_h = 50, color_s = 100, color_b = 100, color_k = 3000;
			if (LIFX.tags[i].color) {
				color_h = LIFX.colors.hue.serialize(LIFX.tags[i].color.hue);
				color_s = LIFX.colors.saturation.serialize(LIFX.tags[i].color.saturation);
				color_b = LIFX.colors.brightness.serialize(LIFX.tags[i].color.brightness);
				color_k = LIFX.colors.kelvin.serialize(LIFX.tags[i].color.kelvin);
			}
			appMessageQueue.send({type:TYPE.TAG, method:METHOD.DATA, index:i, label:label, color_h:color_h, color_s:color_s, color_b:color_b, color_k:color_k});
		}
		appMessageQueue.send({type:TYPE.TAG, method:METHOD.END});
	},

	handleResponse: function(xhr) {
		try {
			var res = JSON.parse(xhr.responseText);
			console.log(JSON.stringify(res));
			switch (LIFX.type) {
				case TYPE.ALL: {
					LIFX.lights = res;
					LIFX.sendLights();
					if (LIFX.tags.length > 0) break;
					LIFX.tags = [];
					LIFX.lights.forEach(function(light) {
						light.tags.forEach(function(tag) {
							if (tag.substring(0,1) == '_') return;
							for (var i = 0; i < LIFX.tags.length; i++) {
								if (LIFX.tags[i].label == tag) return;
							}
							LIFX.tags.push({label:tag, color:light.color});
						});
					});
					LIFX.sendTags();
					break;
				}
				case TYPE.TAG: {
					res.forEach(function(light) {
						for (var i = 0; i < LIFX.lights.length; i++) {
							if (LIFX.lights[i].id == light.id) {
								LIFX.lights[i] = light;
								LIFX.sendLight(i);
							}
						}
					});
					appMessageQueue.send({type:TYPE.LIGHT, method:METHOD.END});
					break;
				}
				case TYPE.LIGHT: {
					for (var i = 0; i < LIFX.lights.length; i++) {
						if (LIFX.lights[i].id == res.id) {
							LIFX.lights[i] = res;
							LIFX.sendLight(i);
						}
					}
					appMessageQueue.send({type:TYPE.LIGHT, method:METHOD.END});
					break;
				}
			}
		} catch(e) {
			console.log(JSON.stringify(e));
			appMessageQueue.send({type:TYPE.ERROR, label:'Error handling response from server!'});
		}
	},

	color: function(hue, saturation, brightness, kelvin) {
		var data = JSON.stringify(this.colors.makePostData(hue, saturation, brightness, kelvin));
		this.makeAPIRequest('PUT', '/color', data, this.handleResponse, this.error);
		setTimeout(function() {
			LIFX.makeAPIRequest('GET', '', null, LIFX.handleResponse, LIFX.error);
		}, 2000);
	},

	toggle: function() {
		this.makeAPIRequest('PUT', '/toggle', null, this.handleResponse, this.error);
	},

	on: function() {
		this.makeAPIRequest('PUT', '/on', null, this.handleResponse, this.error);
	},

	off: function() {
		this.makeAPIRequest('PUT', '/off', null, this.handleResponse, this.error);
	},

	refresh: function() {
		this.type = TYPE.ALL;
		this.makeAPIRequest('GET', '', null, this.handleResponse, this.error);
	},

	makeAPIRequest: function(method, endpoint, data, cb, fb) {
		var url = this.server + '/lights/' + encodeURIComponent(this.getSelector()) + endpoint;
		console.log(method + ' ' + url + ' ' + data);
		var xhr = new XMLHttpRequest();
		xhr.open(method, url, true);
		xhr.onload = function() { cb(xhr); };
		xhr.onerror = function() { fb('Server error!'); };
		xhr.ontimeout = function() { fb('Connection to server timed out!'); };
		xhr.timeout = 30000;
		xhr.send(data);
	}
};

Pebble.addEventListener('ready', function(e) {
	LIFX.refresh();
});

Pebble.addEventListener('appmessage', function(e) {
	console.log('AppMessage received: ' + JSON.stringify(e.payload));
	if (!isset(e.payload.method)) return;
	switch (e.payload.method) {
		case METHOD.REFRESH:
			LIFX.refresh();
			break;
		case METHOD.TOGGLE:
			LIFX.type = e.payload.type;
			LIFX.index = e.payload.index;
			LIFX.toggle();
			break;
		case METHOD.COLOR:
			LIFX.type = e.payload.type;
			LIFX.index = e.payload.index;
			LIFX.color(e.payload.color_h, e.payload.color_s, e.payload.color_b, e.payload.color_k);
			break;
	}
});

Pebble.addEventListener('showConfiguration', function() {
	var data = {server:LIFX.server};
	var uri = 'https://ineal.me/pebble/opalx/configuration/?data=' + encodeURIComponent(JSON.stringify(data));
	console.log('showing configuration at ' + uri);
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
