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
	tags: [],
	tagsSent: false,
	isTagRequest: false,
	index: 255,

	colors: {
		makePostData: function(hue, saturation, brightness) {
			return {hue:LIFX.colors.hue.toReal(hue), saturation:LIFX.colors.saturation.toReal(saturation), brightness:LIFX.colors.brightness.toReal(brightness)};
		},
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

	getSelector: function() {
		if (this.index == 255) return 'all';
		if (this.isTagRequest) return 'tag:' + this.tags[this.index];
		return this.lights[this.index].id;
	},

	handleResponse: function(xhr) {
		try {
			var res = JSON.parse(xhr.responseText);
			if (res instanceof Array) {
				LIFX.lights = [];
				var i = 0;
				for (var r in res) {
					label = res[r].label ? res[r].label.substring(0,18) : res[r].id.substring(0,18);
					state = res[r].on ? 'ON' : 'OFF';
					color_h = LIFX.colors.hue.toFake(res[r].color.hue) || 0;
					color_s = LIFX.colors.saturation.toFake(res[r].color.saturation) || 0;
					color_b = LIFX.colors.brightness.toFake(res[r].color.brightness) || 0;
					appMessageQueue.add({index:i++, label:label, state:state, color_h:color_h, color_s:color_s, color_b:color_b});
					LIFX.lights.push(res[r]);
				}
				appMessageQueue.add({index:i});
				if (!LIFX.tagsSent) {
					LIFX.lights.forEach(function(light) {
						light.tags.forEach(function(tag) {
							LIFX.tags.push(tag);
						});
					});
					LIFX.tags = LIFX.tags.filter(function (e, i, arr) {
						return arr.lastIndexOf(e) === i;
					});
					i = 0;
					for (var t in LIFX.tags) {
						appMessageQueue.add({index:i++, label:LIFX.tags[t], state:'', tag:true});
					}
					appMessageQueue.add({index:i, tag:true});
					LIFX.tagsSent = true;
				}
			} else {
				label = res.label ? res.label.substring(0,18) : res.id.substring(0,18);
				state = res.on ? 'ON' : 'OFF';
				color_h = LIFX.colors.hue.toFake(res.color.hue) || 0;
				color_s = LIFX.colors.saturation.toFake(res.color.saturation) || 0;
				color_b = LIFX.colors.brightness.toFake(res.color.brightness) || 0;
				appMessageQueue.add({index:LIFX.index, label:label, state:state, color_h:color_h, color_s:color_s, color_b:color_b});
				appMessageQueue.add({index:LIFX.lights.length});
			}
		} catch(e) {
			console.log(JSON.stringify(e));
			try {
				var label = LIFX.lights[LIFX.index].label ? LIFX.lights[LIFX.index].label.substring(0,18) : LIFX.lights[LIFX.index].id.substring(0,18);
				appMessageQueue.add({index:LIFX.index, label:label, state:'Err'});
				appMessageQueue.add({index:LIFX.lights.length});
			} catch(ee) {
				appMessageQueue.add({error:'server_error'});
			}
		}
		appMessageQueue.send();
	},

	color: function(hue, saturation, brightness) {
		var data = JSON.stringify(this.colors.makePostData(hue, saturation, brightness));
		this.makeAPIRequest('PUT', '/color.json', data, this.handleResponse, this.error);
	},

	toggle: function() {
		this.makeAPIRequest('PUT', '/toggle.json', null, this.handleResponse, this.error);
	},

	on: function() {
		this.makeAPIRequest('PUT', '/on.json', null, this.handleResponse, this.error);
	},

	off: function() {
		this.makeAPIRequest('PUT', '/off.json', null, this.handleResponse, this.error);
	},

	refresh: function() {
		this.index = 255;
		this.makeAPIRequest('GET', '.json', null, this.handleResponse, this.error);
	},

	makeAPIRequest: function(method, endpoint, data, cb, fb) {
		var url = this.server + '/lights/' + encodeURIComponent(this.getSelector()) + endpoint;
		console.log(method + ' ' + url + ' ' + data);
		var xhr = new XMLHttpRequest();
		xhr.open(method, url, true);
		xhr.onload = function() { cb(xhr); };
		xhr.onerror = function() { fb('error'); };
		xhr.ontimeout = function() { fb('timeout'); };
		xhr.timeout = 10000;
		xhr.send(data);
	}
};

Pebble.addEventListener('ready', function(e) {
	if (!LIFX.server) return LIFX.error('no_server_set');
	LIFX.refresh();
});

Pebble.addEventListener('appmessage', function(e) {
	console.log('AppMessage received: ' + JSON.stringify(e.payload));
	if (isset(e.payload.index)) {
		LIFX.isTagRequest = e.payload.tag || false;
		LIFX.index = e.payload.index;
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
