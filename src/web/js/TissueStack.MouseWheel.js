/*! Copyright (c) 2011 Brandon Aaron (http://brandonaaron.net)
 * Licensed under the MIT License (LICENSE.txt).
 *
 * Thanks to: http://adomas.org/javascript-mouse-wheel/ for some pointers.
 * Thanks to: Mathias Bank(http://www.mathias-bank.de) for a scope bug fix.
 * Thanks to: Seamus Leahy for adding deltaX and deltaY
 *
 * Version: 3.0.6
 * 
 * Requires: 1.2.2+
 */

(function($) {

$.attrFn = $.attrFn || {};

var types = ['DOMMouseScroll', 'mousewheel'];

if ($.event.fixHooks) {
    for ( var i=types.length; i; ) {
        $.event.fixHooks[ types[--i] ] = $.event.mouseHooks;
    }
}

$.event.special.mousewheel = {
    setup: function() {
        if ( this.addEventListener ) {
            for ( var i=types.length; i; ) {
                this.addEventListener( types[--i], handler, false );
            }
        } else {
            this.onmousewheel = handler;
        }
    },
    
    teardown: function() {
        if ( this.removeEventListener ) {
            for ( var i=types.length; i; ) {
                this.removeEventListener( types[--i], handler, false );
            }
        } else {
            this.onmousewheel = null;
        }
    }
};

$.fn.extend({
    mousewheel: function(fn) {
        return fn ? this.bind("mousewheel", fn) : this.trigger("mousewheel");
    },
    
    unmousewheel: function(fn) {
        return this.unbind("mousewheel", fn);
    }
});


	function handler(event) {
	    var orgEvent = event || window.event, args = [].slice.call( arguments, 1 ), delta = 0, deltaX = 0, deltaY = 0;
	    event = $.event.fix(orgEvent);
	    event.type = "mousewheel";
	    
	    // Old school scrollwheel delta
	    if ( orgEvent.wheelDelta ) { delta = orgEvent.wheelDelta/120; }
	    if ( orgEvent.detail     ) { delta = -orgEvent.detail/3; }
	    
	    // New school multidimensional scroll (touchpads) deltas
	    deltaY = delta;
	    
	    // Gecko
	    if ( orgEvent.axis !== undefined && orgEvent.axis === orgEvent.HORIZONTAL_AXIS ) {
	        deltaY = 0;
	        deltaX = -1*delta;
	    }
	    
	    // Webkit
	    if ( orgEvent.wheelDeltaY !== undefined ) { deltaY = orgEvent.wheelDeltaY/120; }
	    if ( orgEvent.wheelDeltaX !== undefined ) { deltaX = -1*orgEvent.wheelDeltaX/120; }
	    
	    // Add event and delta to the front of the arguments
	    args.unshift(event, delta, deltaX, deltaY);
	    
	    return ($.event.dispatch || $.event.handle).apply(this, args);
	}
	
	//DOUBLE TAP TO ENLARGE IMAGES
	
	$.fn.doubletap = function(fn) {
	        return fn ? this.bind('doubletap', fn) : this.trigger('doubletap');
	    };
	
	    $.attrFn.doubletap = true;
	    
	    $.event.special.doubletap = {
	        setup: function(data, namespaces){
	            $(this).bind('touchend', $.event.special.doubletap.handler);
	        },
	
	        teardown: function(namespaces){
	            $(this).unbind('touchend', $.event.special.doubletap.handler);
	        },
	
	        handler: function(event){
	            var action;
	
	            clearTimeout(action);
	
	            var now       = new Date().getTime();
	            var lastTouch = $(this).data('lastTouch') || now + 1;
	            var delta     = now - lastTouch;
	            var delay     = delay == null? 500 : delay;
	
	            if(delta < delay && delta > 0){
	                $(this).data('lastTouch', null);
	                event.type = 'doubletap';
	                $.event.handle.apply(this, arguments);
	            }else{
	                $(this).data('lastTouch', now);
	
	                action = setTimeout(function(evt){
	                    event.type = 'tap';
	                    $.event.handle.apply(this, arguments);
	                    clearTimeout(action);
	                }, delay, [event]);
	            }
	        }
	    };
	
})(jQuery);