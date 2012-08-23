TissueStack.ContrastCanvas = function(element_id) {
	this.element_id = element_id;
	this.initContrastSlider();
	this.registerListeners();
};

TissueStack.ContrastCanvas.prototype = {
	mouse_down: false,
	range : 255, // contrast value range
	margin : 0,  // left hand and right hand margin in between canvas bounds and contrast 'bar'
	step : 0,	 // step size for each contrast step
	width : 0,	 // width of the contrast 'bar' in px
	height: 50,  // height of the contrast 'bar' in px
	start_coords : {x: 0, y: 0}, // the starting coordinate pair (upper left corner) of the contrast bar
	min_bar_pos : -1,	// the position for the min bar position
	max_bar_pos : -1,    // the position for the max bar position
	element_id : null, 	// the canvas element id
	min_or_max : null, // last mouse position was over min or max or nowhere
	close_coords : {x: -1, y: -1}, // coords of the closing x
	getCanvasElement : function() {
		if (typeof(this.element_id) != 'string') return null;
		
		var el = $("#" + this.element_id);
		if (!el || el.length==0) return null;
		else return el;
	},
	getContext : function() {
		var el = this.getCanvasElement();
		if (!el) return null;
		
		return el[0].getContext('2d');
	}, clearCanvas : function() {
		var ctx = this.getContext();
		if (!ctx) return;
		
		ctx.clearRect(0,0, this.getCanvasWidth(), this.getCanvasHeight());
	}, initContrastSlider : function() {
		var ctx = this.getContext();
		if (!ctx) return;
		
		this.step = Math.floor(this.getCanvasWidth() / this.range);
		this.width = this.step * (this.range + 1);
		this.height = 50;
		
		this.margin = Math.floor((this.getCanvasWidth() - this.width) / 2);
		this.start_coords.x = this.margin;
		this.start_coords.y = 50;

		this.drawContrastSlider();
		this.moveBar('min', this.start_coords.x);
		this.moveBar('max', this.start_coords.x + this.width);
		this.drawMinMaxValues();
	}, drawContrastSlider : function() {
		var ctx = this.getContext();
		if (!ctx) return;
		
		this.clearCanvas();
		
		var index = this.range;
		while (index >= 0) {
			ctx.fillStyle = "rgba(" + index + "," + index + "," + index +", 1)";
			ctx.fillRect(this.start_coords.x + (this.range-index)*this.step, this.start_coords.y, this.step, this.height);
			index--;
		}
		
		// surround contrast bar with border
		ctx.beginPath();
		ctx.strokeStyle = "rgba(255,255,255,1)";
		ctx.lineWidth = 2;
		ctx.moveTo(this.start_coords.x,this.start_coords.y);
		ctx.lineTo(this.start_coords.x+this.width,this.start_coords.y);
		ctx.lineTo(this.start_coords.x+this.width,this.start_coords.y+this.height);
		ctx.lineTo(this.start_coords.x,this.start_coords.y+this.height);
		ctx.lineTo(this.start_coords.x,this.start_coords.y);
		ctx.stroke();
		
		this.drawClosingX();
	}, drawClosingX : function() {
		var ctx = this.getContext();
		if (!ctx) return;
		
		var _this = this;
		
		var close = new Image();
		close.src = '/images/close.png';
		this.close_coords.x = _this.start_coords.x + _this.width - 35;
		this.close_coords.y = _this.start_coords.y - 35;
		
		close.onload = function() {
			ctx.drawImage(this, _this.close_coords.x, _this.close_coords.y);
		};
	}, drawMinMaxValues : function() {
		var ctx = this.getContext();
		if (!ctx) return;

		ctx.font = "15pt Calibri";
		ctx.fillStyle = "rgba(0, 0, 200, 1)";
		ctx.fillText("Min: " + this.getMinimum(), this.start_coords.x + (this.width / 4) - 50, 40);
		ctx.fillStyle = "rgba(200, 0, 0, 1)";
		ctx.fillText("Max: " + this.getMaximum(), this.start_coords.x + (this.width / 4) + 50, 40);
	},
	getCanvasWidth : function() {
		var el = this.getCanvasElement(); 
		if (!el) return 0;
		else return el.width();
	},
	getCanvasHeight : function() {
		var el = this.getCanvasElement(); 
		if (!el) return 0;
		else return el.height();
	},
	moveBar : function(which, position) {
		if (typeof(which) != "string" || typeof(position) != "number") return;

		if (position < this.start_coords.x) position =  this.start_coords.x;
		if (position > this.start_coords.x + this.width - this.step) {
			position =  this.start_coords.x + this.width - this.step;	
		}
		
		// make sure that min and max don't conflict
		if ((this.min_bar_pos > 0 && which === 'min' && position + this.step >= this.max_bar_pos)
				|| (this.max_bar_pos > 0 && which === 'max' && position -this.step <= this.min_bar_pos)) {
			var other = which === 'min' ? 'max' : 'min';
			position = this[other + "_bar_pos"] + (which === 'min' ? -this.step : this.step);
		} 
		
		this[which + '_bar_pos'] = position;
		
		var ctx = this.getContext();
		if (!ctx) return;
		
		// draw bar
		ctx.fillStyle = (which == 'min' ? "rgba(0, 0, 200, 1)" : "rgba(200, 0, 0, 1)");
		ctx.fillRect(this[which + "_bar_pos"], this.start_coords.y, this.step, this.height);
	},
	registerListeners : function() {
		if (!this.getCanvasElement()) return;
		
		var _this = this;
		
		// avoid potential double binding
		this.unregisterListeners();
		
		// CLICK
		this.getCanvasElement().bind("click", function(e) {
			if (e.originalEvent.touches) {
				var touches = e.originalEvent.touches[0] || e.originalEvent.changedTouches[0];
				e.pageX = touches.pageX;
				e.pageY = touches.pageY;
			}
			
			var coords = TissueStack.Utils.getRelativeMouseCoords(e);
			
			// do we want to close
			if (coords.x >= _this.close_coords.x && coords.x <= _this.close_coords.x + 30 
					&& coords.y >= _this.close_coords.y && coords.y <= _this.close_coords.y + 30) {
				_this.hideContrastCanvas();
			}
		});
		
		// MOUSE UP
		this.getCanvasElement().bind("mouseup", function(e) {
			_this.mouse_down = false;
		});
		//MOUSE DOWN
		this.getCanvasElement().bind("mousedown", function(e) {
			var coords = TissueStack.Utils.getRelativeMouseCoords(e);
			_this.min_or_max = _this.isMinOrMaxMove(coords);
			if (TissueStack.Utils.isLeftMouseButtonPressed(e) && _this.min_or_max) _this.mouse_down = true;
		});			
		// MOUSE MOVE
		this.getCanvasElement().bind("mousemove", function(e) {
			var coords = TissueStack.Utils.getRelativeMouseCoords(e);
			if (_this.mouse_down) {
				_this.drawContrastSlider();
				_this.moveBar(_this.min_or_max, coords.x);
				var other = (_this.min_or_max === "max" ? "min" : "max");
				_this.moveBar(other, _this[other + "_bar_pos"]);
				_this.drawMinMaxValues();
			}
		});
	},
	unregisterListeners : function() {
		if (!this.getCanvasElement()) return;
		
		this.getCanvasElement().unbind("mouseup");
		this.getCanvasElement().unbind("mousedown");
		this.getCanvasElement().unbind("mousemove");
		this.getCanvasElement().unbind("click");
	},
	isMinOrMaxMove : function(coords) {
		if (coords.x >= this.min_bar_pos && coords.x <= this.min_bar_pos + this.step) {
			return "min";
		} else if (coords.x >= this.max_bar_pos && coords.x <= this.max_bar_pos + this.step) {
			return "max";
		}
		
		return null;
	},getMinimum : function() {
		return Math.floor((this.min_bar_pos - this.margin) /this.step);
	}
	,getMaximum : function() {
		return Math.floor((this.max_bar_pos - this.margin) /this.step);
	}, hideContrastCanvas : function() {
		if (!this.getCanvasElement()) return;
		
		this.getCanvasElement().hide();
	}
};
