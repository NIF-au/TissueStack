/*
 * This file is part of TissueStack.
 *
 * TissueStack is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * TissueStack is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with TissueStack.  If not, see <http://www.gnu.org/licenses/>.
 */
TissueStack.ContrastCanvas = function(element_id, min, max) {
	if (typeof(element_id) != "string") return;

	this.element_id = element_id;
	this.dataset_min = typeof(min) === 'number' ? min : 0;
	this.dataset_max = typeof(max) === 'number' ? max : 255;
	this.initContrastSlider();
	this.registerListeners();
	this.registerAdditionalPhoneEvents();
};

TissueStack.ContrastCanvas.prototype = {
	canvas: null,
	mouse_down: false,
	range : 255, // contrast value range
	margin : 0,  // left hand and right hand margin in between canvas bounds and contrast 'bar'
	step : 0,	 // step size for each contrast step
	width : 0,	 // width of the contrast 'bar' in px
	height: 75,  // height of the contrast 'bar' in px
	start_coords : {x: 0, y: 0}, // the starting coordinate pair (upper left corner) of the contrast bar
	min_bar_pos : -1,	// the position for the min bar position
	max_bar_pos : -1,    // the position for the max bar position
	element_id : null, 	// the canvas element id
	min_or_max : null, // last mouse position was over min or max or nowhere
	dataset_min : 0,	// the dataset's min value
	dataset_max : 255, 	// the dataset's max value
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
		this.height = 75;

		this.margin = Math.floor((this.getCanvasWidth() - this.width) / 2);
		this.start_coords.x = this.margin;
		this.start_coords.y = 50;

		this.drawContrastSlider();
		this.moveBar('min', this.start_coords.x + this.dataset_min * this.step);
		this.moveBar('max', this.start_coords.x + this.dataset_max * this.step);
		this.drawMinMaxValues();
	}, drawContrastSlider : function() {
		var ctx = this.getContext();
		if (!ctx) return;

		this.clearCanvas();

		var index = this.range;
		while (index >= 0) {
			ctx.fillStyle = "rgba(" + index + "," + index + "," + index +", 1)";
			ctx.fillRect(this.start_coords.x + index*this.step, this.start_coords.y, this.step, this.height);
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
	}, drawMinMaxValues : function() {
		var ctx = this.getContext();
		if (!ctx) return;

		ctx.font = "12pt Calibri";
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
		if ((this.min_bar_pos > 0 && which === 'min' && position + this.step * 2 >= this.max_bar_pos)
				|| (this.max_bar_pos > 0 && which === 'max' && position - this.step * 2 <= this.min_bar_pos)) {
			var other = which === 'min' ? 'max' : 'min';
			position = this[other + "_bar_pos"] + (which === 'min' ? - this.step : this.step);
		}

		this[which + '_bar_pos'] = position;

		var ctx = this.getContext();
		if (!ctx) return;

		// draw bar
		ctx.fillStyle = (which == 'min' ? "rgba(0, 0, 200, 1)" : "rgba(200, 0, 0, 1)");
		ctx.fillRect(this[which + "_bar_pos"], this.start_coords.y, this.step * 2, this.height);
	},
	registerListeners : function() {
		if (!this.getCanvasElement()) return;

		var _this = this;

		// avoid potential double binding
		this.unregisterListeners();

		// DESKTOP
		if (TissueStack.desktop || TissueStack.debug) {
			// MOUSE UP
			this.getCanvasElement().bind("mouseup", function(e) {
				_this.mouse_down = false;
				_this.canvas.events.changeSliceForPlane(_this.canvas.getDataExtent().slice);
				_this.canvas.events.updateCoordinateDisplay();
			});
			// MOUSE DOWN
			this.getCanvasElement().bind("mousedown", function(e) {
				var coords = TissueStack.Utils.getRelativeMouseCoords(e);
				_this.min_or_max = _this.isMinOrMaxMove(coords);
				if (TissueStack.Utils.isLeftMouseButtonPressed(e) && _this.min_or_max) _this.mouse_down = true;
			});
			// MOUSE MOVE
			this.getCanvasElement().bind("mousemove", function(e) {
				var coords = TissueStack.Utils.getRelativeMouseCoords(e);
				if (_this.mouse_down) {
					_this.makeTouchMouseMove(coords.x);
				}
			});
		};

		// TABLET & PHONE
		if(TissueStack.tablet || TissueStack.phone || TissueStack.debug) {
			// TOUCH END
			this.getCanvasElement().bind("touchend", function(e) {
				_this.mouse_down = false;
				if(TissueStack.tablet || TissueStack.debug){
					_this.canvas.events.changeSliceForPlane(_this.canvas.getDataExtent().slice);
					_this.canvas.events.updateCoordinateDisplay();
				}
			});
			// TOUCH START
			this.getCanvasElement().bind("touchstart", function(e) {
				_this.mouse_down = true;
			});
			// TOUCH MOVE
			this.getCanvasElement().bind("touchmove", function(e) {
				if (e.originalEvent.touches) {
						var touches = e.originalEvent.touches[0] || e.originalEvent.changedTouches[0];
						e.pageX = touches.pageX;
						e.pageY = touches.pageY;
				}

				var coords = TissueStack.Utils.getRelativeMouseCoords(e);
				_this.min_or_max = _this.isMinOrMaxMove(coords);

				if(_this.mouse_down) _this.makeTouchMouseMove(coords.x);
			});
		};
	},
	unregisterListeners : function() {
		if(TissueStack.phone) {
			$('#contrast_button, #reset_contrast_button').unbind("click");
		};

		if (!this.getCanvasElement()) return;

		this.getCanvasElement().unbind("touchend mouseup");
		this.getCanvasElement().unbind("touchstart mousedown");
		this.getCanvasElement().unbind("touchmove mousemove");
	},
	isMinOrMaxMove : function(coords) {
		if(TissueStack.desktop){
			if (coords.x >= this.min_bar_pos - this.step * 2 && coords.x <= this.min_bar_pos + this.step * 5) {
				return "min";
			} else if (coords.x >= this.max_bar_pos - this.step * 2 && coords.x <= this.max_bar_pos + this.step * 5) {
				return "max";
			}
		}
		if(TissueStack.tablet || TissueStack.phone || TissueStack.debug){
			if (coords.x >= this.min_bar_pos - this.step * 30 && coords.x <= this.min_bar_pos + this.step * 30) {
				return "min";
			} else if (coords.x >= this.max_bar_pos - this.step * 30 && coords.x <= this.max_bar_pos + this.step * 30) {
				return "max";
			}
		}
		return null;
	},
	getMinimum : function() {
        if (this.step == 0) return 0;
		return Math.floor((this.min_bar_pos - this.margin) /this.step);
	},
	getMaximum : function() {
        if (this.step == 0) return 255;
		return Math.floor((this.max_bar_pos - this.margin) /this.step);
	},
	getMinimumBarPositionForValue : function(val) {
		return this.min_bar_pos = this.margin + val * this.step;
	},
	getMaximumBarPositionForValue : function(val) {
		return this.max_bar_pos =this.margin + val * this.step;
	},
	makeTouchMouseMove: function (coords) {
	 	var _this = this;

		_this.drawContrastSlider();
		_this.moveBar(_this.min_or_max, coords);
		var other = (_this.min_or_max === "max" ? "min" : "max");
		_this.moveBar(other, _this[other + "_bar_pos"]);
		_this.drawMinMaxValues();
	},
	registerAdditionalPhoneEvents : function() {
		if(!TissueStack.phone) return;

		var _this = this;
		$('#contrast_button').click(function(){
			if (_this.canvas) _this.canvas.events.changeSliceForPlane(_this.canvas.getDataExtent().slice);
		});
		$('#reset_contrast_button').click(function(){
			_this.initContrastSlider();
			if (_this.canvas) _this.canvas.events.changeSliceForPlane(_this.canvas.getDataExtent().slice);
		});
	},
	isMinOrMaxDifferentFromDataSetMinOrMax : function() {
		if (this.dataset_min == this.getMinimum() && this.dataset_max == this.getMaximum()) return false;

		return true;
	}
};
