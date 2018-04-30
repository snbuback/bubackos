'use strict';

var console = function() {
    // logging levels from kernel/logging.h
    return {
        debug: function(msg) {
            platform.logging(1, msg);
        },
        info: function(msg) {
            platform.logging(2, msg);
        },
        warn: function(msg) {
            platform.logging(3, msg);
        },
        error: function(msg) {
            platform.logging(4, msg);
        }
    }
}();
