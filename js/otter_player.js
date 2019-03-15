// 供C调用，获取远程文件大小
function JS_XHRGetFileSize(loadPtr, url) {
    var request = new XMLHttpRequest();
    var size = 0;

    request.open("GET", url, true);
    request.responseType = "arraybuffer";
    request.url = url + "?" + Math.random();
    request.setRequestHeader("Range", "bytes=0-187");
    request.onreadystatechange = function () {
        if (request.readyState == 4) {
            if (request.status == 206) {
                var content_range = request.getResponseHeader("Content-Range");
                if (content_range != undefined) {
                    size = content_range.split("/")[1];
                }
                console.log("C CALL JS, JS_XHRGetFileSize success, size:" + size);
                Module.ccall('_xhr_on_file_size_success', 'null', ['number', 'number'], [loadPtr, size]);
            }
            else {
                console.log("C CALL JS, JS_XHRGetFileSize failed!");
                Module.ccall('_xhr_on_file_size_success', 'null', ['number', 'number'], [loadPtr, 0]);
            }
        }
    };
    request.send();
}

function JS_XHRGetFile(loadPtr, url, start, end) {
    var request = new XMLHttpRequest();
    var range = "bytes=" + start + "-" + (end -1);
    var len = 0;
    request.open("GET", url, true);
    request.responseType = "arraybuffer";
    request.url = url + "?" + Math.random();
    request.setRequestHeader("Range", range);
    request.onreadystatechange = function () {
        if (request.readyState == 4) {
            if (request.status == 206) {

                var arrayBuffer = request.response;
                if (arrayBuffer) {
                    var byteArray = new Uint8Array(arrayBuffer);
                    len = byteArray.byteLength;

                    var ptr = Module._malloc(len);
                    for (var i = 0; i < len; i++) {
                        Module.HEAP8[ptr + i] = byteArray[i];
                    }

                    console.log("C CALL JS, JS_XHRGetFile success");
                    Module.ccall('_xhr_on_load_success', 'null', ['number', 'number', 'number'], [loadPtr, ptr, len]);

                    console.log("JS_XHRGetFile free data ptr");
                    Module._free(ptr);
                } else {
                    console.log("C CALL JS, JS_XHRGetFile success but no data");
                    Module.ccall('_xhr_on_load_success', 'null', ['number', 'number', 'number'], [loadPtr, 0, len]);
                }
            }
            else {
                console.log("C CALL JS, JS_XHRGetFile failed!");
                Module.ccall('_xhr_on_load_success', 'null', ['number', 'number', 'number'], [loadPtr, 0, len]);
            }
        }
    };
    request.send();
}


// JS player类定义
function _player(c_player) {
    this.c_player = c_player;
    this.set_media = function (url, duration) {
        if (!url) {
            console.error("url is undefined!");
            return;
        }
        if (!duration) {
            console.error("duration is undefined!");
            return;
        }
        var ptr = allocate(intArrayFromString(url), 'i8', ALLOC_NORMAL);
        Module._set_media(this.c_player, ptr, duration);
        _free(ptr);
        return;
    };
    this.play = function () {
        Module._play(this.c_player, 0);
    };
    this.seek = function (time) {
        Module._play_by_time(this.c_player, time);
    };
    this.stop = function () {
        Module._stop(this.c_player);
    };
}

// otter_player
var otter_player = {
    create_player: function (width, height) {
        var c_player = Module._create_player(width, height);

        if (!c_player) {
            return null;
        }

        return  new _player(c_player);
    }
};
