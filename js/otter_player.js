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
                Module.ccall('_xhr_on_file_size_success', 'null', ['number', 'string'], [loadPtr, size]);
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
    var range = "bytes=" + start + "-" + end;
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

                    Module.ccall('_xhr_on_load_success', 'null', ['number', 'number', 'number'], [loadPtr, ptr, len]);
                } else {
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
    this.audio_player;

    this.init = function () {
        this.current_time = 0;
        this.audio_player = new this.AUDIO_PLAYER(this.c_player);
        this.audio_player._init();
    };

    this.AUDIO_PLAYER = function (c_player) {
        this.c_player = c_player;
        this.audio_ctx;
        this.audio_buffer_0 = -1;
        this.audio_buffer_1 = -1;
        this.source_0;
        this.source_1;
        this.main_buffer_using = false;
        this._init = function () {
            let AudioContext = window.AudioContext || window.webkitAudioContext;
            this.audio_ctx = AudioContext ? new AudioContext() : '';

        },
        this._prepare_buffer = function () {

            if (this.main_buffer_using) {
                this.source_1 = undefined;
            } else {
                this.source_0 = undefined;
            }

            var jframePtr = Module._js_poll_frame(this.c_player);

            if (jframePtr == 0)
            {
                setTimeout(function () { this._playSound() }.bind(this), 1000);
                return -1;
            }

            let len = Module.HEAPU32[jframePtr >> 2];
            let cur_time = Module.HEAPU32[(jframePtr >> 2) + 1];
            let av_type = Module.HEAPU32[(jframePtr >> 2) + 2];
            let channels = Module.HEAP32[(jframePtr >> 2) + 3];
            let dataPtr = Module.HEAP32[(jframePtr >> 2) + 4];

            if (av_type != 0) {
                Module._free(dataPtr);
                Module._free(jframePtr);
                this._playSound();
                return -1;
            }

            // 数据字节数
            var data_byte_count = len / 4; // 2048字节  两个声道

            // 样本帧数 8192 /4 2048 /2 = 1024
            var frame_count = data_byte_count / channels; // 单声道帧数
          //  console.log("bufer_count:" + frame_count);
            var audio_buffer = this.audio_ctx.createBuffer(channels, frame_count, this.audio_ctx.sampleRate);

            // 遍历帧 1-1024
            for (k = 0; k < frame_count; k++) {

                // 声道 2
                for (var channel = 0; channel < 2; channel++) {
                    var nowBuffering = audio_buffer.getChannelData(channel);
                    nowBuffering[k] = Module.HEAPF32[(dataPtr >> 2) + k * 2 + channel];
                }
            }

            if (this.main_buffer_using) {
                this.audio_buffer_1 = audio_buffer;
                var source = this.audio_ctx.createBufferSource();
                var gain_node = this.audio_ctx.createGain();
                gain_node.connect(this.audio_ctx.destination);
                source.connect(gain_node);
                source.buffer = this.audio_buffer_1;
                source.addEventListener('ended', (event) => {
                    //console.log("source_1 ended")
                    this.main_buffer_using = !this.main_buffer_using;
                    this._playSound();
                });
             
                this.source_1 = source;
            } else {
                this.audio_buffer_0 = audio_buffer;
                var source = this.audio_ctx.createBufferSource();
                var gain_node = this.audio_ctx.createGain();
                gain_node.connect(this.audio_ctx.destination);
                source.connect(gain_node);
                source.buffer = this.audio_buffer_0;
                source.addEventListener('ended', (event) => {
                   // console.log("source_0 ended")
                    this.main_buffer_using = !this.main_buffer_using;
                    this._playSound();
                });
                this.source_0 = source;
            }

            Module._free(dataPtr);
            Module._free(jframePtr);
          //  console.log("prepare buffer end");
            return 0;
        };
        this._playSound = function () {
            if (this.audio_ctx == '') {
                return;
            }

            if (this.audio_ctx.state != 'running') {
                this.audio_ctx.resume();
            }

            if (this.audio_buffer_0 == -1)
            {
                if (this._prepare_buffer() < 0)
                {
                    return;
                }
                this.main_buffer_using = !this.main_buffer_using;
            }
            if (this._prepare_buffer() < 0) {
                return;
            }
            if (this.main_buffer_using) {

                if (this.source_0)
                {
                   // console.log("source_0 start")
                    this.source_0.start(0);
                }

               
            } else {
              
                if (this.source_1) {
                   // console.log("source_1 start")
                    this.source_1.start(0);
                }
            }
        }
    };

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
        Module._play_or_seek(this.c_player, 0);
        this.audio_player._playSound();
        return;
    };
    this.seek = function (time) {
        Module._play_or_seek(this.c_player, time);
    };
    this.stop = function () {
        Module._destroy_player(this.c_player);
    };
   
    this.test_poll = function () {
        let AudioContext = window.AudioContext || window.webkitAudioContext;
        var audio_ctx = AudioContext ? new AudioContext() : '';

        console.log(audio_ctx.sampleRate);
        var size = 0;
        var ptrs = []
        for (var i = 0; i < 10000; i++) {

            var jframePtr = Module._js_poll_frame(this.c_player);
            if (jframePtr == 0) {
                continue;
            }
      
            let len = Module.HEAP32[jframePtr >> 2];
            let cur_time = Module.HEAP32[(jframePtr >> 2) + 1];
            let av_type = Module.HEAP32[(jframePtr >> 2) + 2];
            let channels = Module.HEAP32[(jframePtr >> 2) + 3];
            let dataPtr = Module.HEAP32[(jframePtr >> 2) + 4];
            if (av_type != 0) {
                continue;
            }
            ptrs.push({ ptr: dataPtr, len: len });
            size = size + len;
        }

        // 样本帧数 172032
        var bufer_count = size / 4 /2;
        var audio_buffer = audio_ctx.createBuffer(2, bufer_count, audio_ctx.sampleRate);
        console.log("bufer_count:" + bufer_count);
        var index = 0;
        console.log("ptrs.length:" + ptrs.length);
        for (var j = 0; j < ptrs.length; j++) {
            var data_byte_count =  ptrs[j].len / 4; // 2048
            var single_channel_frame_count = data_byte_count / 2;
            var dataPtr = ptrs[j].ptr;

            // 遍历帧 1-1024
            for (k = 0; k < single_channel_frame_count; k++) {

                // 声道 2
                for (var channel = 0; channel < 2; channel++) {
                    var nowBuffering = audio_buffer.getChannelData(channel);
                    nowBuffering[index + k] = Module.HEAPF32[(dataPtr >> 2) + k * 2 + channel];
                }
            }
            index = index + single_channel_frame_count;
        }

        var source = audio_ctx.createBufferSource();
        source.connect(audio_ctx.destination);
        source.buffer = audio_buffer;
        source.start(0);
    }
}

// otter_player
var otter_player = {
    create_player: function (width, height) {
        var c_player = Module._create_player(width, height);
        if (!c_player) {
            return null;
        }
        var player = new _player(c_player);
        player.init();
        return player;
    }
};
