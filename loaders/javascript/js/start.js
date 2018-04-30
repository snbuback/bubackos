'use strict';
/**
 * Global objects are slow to call, so building the object as local before set as global
 */
var OS = function() {
    return {
        start: function() {
            console.debug('total memory=' + (platform.getTotalMemory()/1024/1024) + 'MB');
            console.info('js engine working :-)');
        }
    }
}();
