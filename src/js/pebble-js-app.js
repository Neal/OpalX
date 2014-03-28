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
	END: 2
};

var LIFX = {
	server: localStorage.getItem('server') || '',
	lights: [],
	tags: [],
	type: null,
	method: null,
	index: 0,

	colors: {
		makePostData: function(hue, saturation, brightness) {
			return {hue:this.hue.deserialize(hue), saturation:this.saturation.deserialize(saturation), brightness:this.brightness.deserialize(brightness)};
		},
		hue: {
			serialize: function(val) { return parseInt((val / 360) * 100); },
			deserialize: function(val) { return val * 3.6; }
		},
		saturation: {
			serialize: function(val) { return parseInt(val * 100); },
			deserialize: function(val) { return val / 100; }
		},
		brightness: {
			serialize: function(val) { return parseInt(val * 100); },
			deserialize: function(val) { return val / 100; }
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
				return 'tag:' + this.tags[this.index];
		}
	},

	sendLight: function(index) {
		label = this.lights[index].label ? this.lights[index].label.substring(0,18) : this.lights[index].id;
		state = this.lights[index].on ? 'ON' : 'OFF';
		color_h = LIFX.colors.hue.serialize(this.lights[index].color.hue) || 0;
		color_s = LIFX.colors.saturation.serialize(this.lights[index].color.saturation) || 0;
		color_b = LIFX.colors.brightness.serialize(this.lights[index].color.brightness) || 0;
		appMessageQueue.send({type:TYPE.LIGHT, method:METHOD.DATA, index:index, label:label, state:state, color_h:color_h, color_s:color_s, color_b:color_b});
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
			appMessageQueue.send({type:TYPE.TAG, method:METHOD.DATA, index:i, label:LIFX.tags[i]});
		}
		appMessageQueue.send({type:TYPE.TAG, method:METHOD.END});
	},

	handleResponse: function(xhr) {
		try {
			var res = JSON.parse(xhr.responseText);
			console.log(JSON.stringify(res));
			if (res instanceof Array) {
				LIFX.lights = res;
				LIFX.sendLights();
				if (LIFX.tags.length === 0) {
					LIFX.tags = [];
					LIFX.lights.forEach(function(light) {
						light.tags.forEach(function(tag) {
							LIFX.tags.push(tag);
						});
					});
					LIFX.tags = LIFX.tags.filter(function (e, i, arr) {
						return arr.lastIndexOf(e) === i;
					});
					LIFX.sendTags();
				}
			} else {
				for (var i = 0; i < LIFX.lights.length; i++) {
					if (LIFX.lights[i].id == res.id) break;
				}
				LIFX.lights[i] = res;
				LIFX.sendLight(i);
				appMessageQueue.send({type:TYPE.LIGHT, method:METHOD.END});
			}
		} catch(e) {
			console.log(JSON.stringify(e));
			appMessageQueue.send({type:TYPE.ERROR, label:'Server error!'});
		}
	},

	color: function(hue, saturation, brightness) {
		var data = JSON.stringify(this.colors.makePostData(hue, saturation, brightness));
		this.makeAPIRequest('PUT', '/color', data, this.handleResponse, this.error);
		setTimeout(function() { LIFX.refresh(); }, 2000);
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
		xhr.ontimeout = function() { fb('Connection timed out!'); };
		xhr.timeout = 10000;
		xhr.send(data);
	}
};

Pebble.addEventListener('ready', function(e) {
	if (!LIFX.server) return LIFX.error('No server set');
	LIFX.refresh();
});

Pebble.addEventListener('appmessage', function(e) {
	console.log('AppMessage received: ' + JSON.stringify(e.payload));
	if (isset(e.payload.index)) {
		LIFX.type = e.payload.type || null;
		LIFX.index = e.payload.index || 0;
		if (isset(e.payload.color_h) && isset(e.payload.color_s) && isset(e.payload.color_b)) {
			LIFX.color(e.payload.color_h, e.payload.color_s, e.payload.color_b);
		} else {
			LIFX.toggle();
		}
	} else {
		LIFX.refresh();
	}
});

Pebble.addEventListener('showConfiguration', function() {
	var uri = 'https://rawgithub.com/Neal/PebblFX/master/configuration/index.html?server=' + encodeURIComponent(LIFX.server);
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
