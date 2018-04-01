'use strict';
/**
 * Global objects are slow to call, so building the object as local before set as global
 */
var console = function() {
    return {
        info: function(msg) {
            print('Console INFO :' + msg);
        }
    }
}();
print('console loaded');
