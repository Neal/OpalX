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
	error: function(error) {
		appMessageQueue.clear();
		appMessageQueue.add({error:error});
		appMessageQueue.send();
	},
	makeColor: function(color) {
		try {
			return parseInt(color.hue) + ' · ' + parseInt(color.saturation) + ' · ' + parseInt(color.brightness);
		} catch (e) {
			return 'Unknown';
		}
	},
	toggle: function(index) {
		if (!this.server) return this.error('no_server_set');
		var xhr = new XMLHttpRequest();
		xhr.open('PUT', 'http://' + this.server + ':3000/lights/' + encodeURIComponent(this.devices[index].id) + '/toggle.json', true);
		xhr.onload = function() {
			appMessageQueue.clear();
			try {
				var res = JSON.parse(xhr.responseText);
				console.log('toggle: ' + JSON.stringify(res));
				var label = res.label.substring(0,32) || res.id.substring(0,32) || '';
				var color = LIFX.makeColor(res.color) || '';
				var state = res.on ? 'ON' : 'OFF';
				appMessageQueue.add({index:index, label:label, color:color, state:state});
				appMessageQueue.add({index:LIFX.devices.length});
			} catch(e) {
				appMessageQueue.add({index:index, label:LIFX.devices[index].label, color:'Error!', state:''});
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
		xhr.open('GET', 'http://' + this.server + ':3000/lights.json', true);
		xhr.onload = function() {
			appMessageQueue.clear();
			try {
				var res = JSON.parse(xhr.responseText);
				var i = 0;
				for (var r in res) {
					var label = res[r].label.substring(0,32) || res[r].id.substring(0,32) || '';
					var color = LIFX.makeColor(res[r].color) || '';
					var state = res[r].on ? 'ON' : 'OFF';
					appMessageQueue.add({index:i++, label:label, color:color, state:state});
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
	if (typeof(e.payload.index) != 'undefined') {
		LIFX.toggle(e.payload.index);
	} else{
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
