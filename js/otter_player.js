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
                console.error("C CALL JS, JS_XHRGetFileSize failed!");
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
                console.error("C CALL JS, JS_XHRGetFile failed!");
                Module.ccall('_xhr_on_load_success', 'null', ['number', 'number', 'number'], [loadPtr, 0, len]);
            }
        }
    };
    request.send();
}

// JS player类定义
function _player(c_player) {
    this.c_player = c_player;
    this.canvasElem;
    this.audio_player;
    this.c_player = c_player;
    this.MERGE_COUNT = 48;
    this.audio_ctx;
    this.retry = !1;
    this.last_duration = 0; // 上一帧时长
    this.last_start_time = 0; // 上一帧开始时间
    this.start_time = 0;
    this.last_try_time = 0;
    this.finish = 0;
    this.frame_duration = 0;
    this.audio_frame_num = 0;
    this.played_audio_frame_num = 0;
    this.arr_idx = 0;
    this.frame_arr = new Array(50);
    this.canvas_ctx;
    this.witdh = 0;
    this.height = 0;
    this.init = function (width, height, canvasElem) {
        this.current_time = 0;
        let AudioContext = window.AudioContext || window.webkitAudioContext;
        this.audio_ctx = AudioContext ? new AudioContext() : '';
        this.witdh = width;
        this.height = height;

        this.status = 0;

        this.canvasElem = canvasElem;
        this.canvas_ctx = this.canvasElem.getContext("2d");

        let imgData = this.canvas_ctx.createImageData(this.canvasElem.width, this.canvasElem.height);
        for (var i = 0; i < imgData.data.length; i = i + 4) {
            imgData.data[i] = 0;
            imgData.data[i + 1] = 0;
            imgData.data[i + 2] = 0;
            imgData.data[i + 3] = 255;
        }
        this.canvas_ctx.putImageData(imgData, 0, 0);
    };

    this._render = function () {

        if (this.finish) {
            console.log("audio finish!");
            return;
        }
        var now = performance.now();
        if (this.last_start_time != 0 && (this.last_start_time + this.last_duration - now) < -5000) {
            this.finish = 1;
        }
        if (this.start_time != 0) {				
			
			for (i = 0; i < this.frame_arr.length; i++) {
				if (!this.frame_arr[i]) {
					continue;
				}					
				
	
					var imgData = this.frame_arr[i].data;						
					this.canvas_ctx.putImageData(imgData, 0, 0);				
					this.frame_arr[i] = undefined;	
					break;
			
			}
         
        }

        if (this.retry) {
            if ((now - this.last_try_time) > 200) {
                this.retry = !1;
                this.last_try_time = 0;
                this._prepare_source();
            }
        } else {

            // 上一个起播 100ms之后，准备下一个
            if ((this.last_start_time + this.last_duration - now) <= this.last_duration / 2) {
                this._prepare_source();
            }
        }

        var id = requestAnimationFrame(function () { this._render() }.bind(this));
    },
    this._prepare_source = function () {

        // console.log("prepare in!");
        var jframe_arr = new Array(this.MERGE_COUNT);
        var jframe_count = 0; // 多少个js音频帧
        var data_byte_count = 0;
        var total_frame_count = 0; // 单声道帧数
        var audio_buffer;
        var channels;
        for (; jframe_count < this.MERGE_COUNT;) {
            var jframePtr = Module._js_poll_frame(this.c_player);

            if (jframePtr == 0) {
                break;
            }

            var frame_item = new Object();

            // 数据长度
            frame_item.len = Module.HEAPU32[jframePtr >> 2];

            // 当前时间
            frame_item.cur_time = Module.HEAPU32[(jframePtr >> 2) + 1];

            // 帧类型
            frame_item.av_type = Module.HEAPU32[(jframePtr >> 2) + 2];

            // 音频频道数
            frame_item.channels = Module.HEAP32[(jframePtr >> 2) + 3];

            // 数据指针
            frame_item.dataPtr = Module.HEAP32[(jframePtr >> 2) + 4];

            // 视频
            if (frame_item.av_type == 1) {
                
    
				var imgData = this.canvas_ctx.createImageData(480, 320);
				var i = 0, imgIdx = 0;
				for (; imgIdx < imgData.data.length;) {
					imgData.data[imgIdx] = Module.HEAPU8[frame_item.dataPtr + i];
					imgData.data[imgIdx + 1] = Module.HEAPU8[frame_item.dataPtr + i + 1];
					imgData.data[imgIdx + 2] = Module.HEAPU8[frame_item.dataPtr + i + 2];
					imgData.data[imgIdx + 3] = 255;
					imgIdx = imgIdx + 4;
					i = i + 3;
				}	

				var frame_data = { data: imgData, time: frame_item.cur_time };
				Module._free(frame_item.dataPtr);				
                this.frame_arr[this.arr_idx % this.frame_arr.length] = frame_data;
				this.arr_idx++;
               
            } else {

                // 样本帧数
                channels = frame_item.channels;
                total_frame_count += frame_item.len / 4 / frame_item.channels; // 单声道帧数
                jframe_arr[jframe_count] = frame_item;
                jframe_count++;
                this.audio_frame_num++;
            }
            Module._free(jframePtr);
        }

        if (jframe_count == 0) {
            this.retry = !0;
            this.last_try_time = performance.now();
            return;
        }

        audio_buffer = this.audio_ctx.createBuffer(channels, total_frame_count, this.audio_ctx.sampleRate);

        let idx = 0;
        for (var i = 0; i < jframe_count; i++) {
            let frame_item = jframe_arr[i];
            let frame_count = frame_item.len / 4 / frame_item.channels;

            // 遍历帧 1-1024
            for (k = 0; k < frame_count; k++) {

                // 声道 2
                for (var channel = 0; channel < 2; channel++) {
                    var nowBuffering = audio_buffer.getChannelData(channel);
                    nowBuffering[k + idx] = Module.HEAPF32[(frame_item.dataPtr >> 2) + k * 2 + channel];
                }
            }
            idx += frame_count;
            Module._free(frame_item.dataPtr);
        }

        var source = this.audio_ctx.createBufferSource();
        var gain_node = this.audio_ctx.createGain();
        gain_node.connect(this.audio_ctx.destination);
        source.connect(gain_node);
        source.buffer = audio_buffer;

        var future_time;

        // 当前为第一时间片
        var now = performance.now();
        if (this.last_start_time == 0) {
            future_time = 0;
            this.last_start_time = now;
            this.last_duration = audio_buffer.duration * 1000;
            this.start_time = now;
        } else {
            future_time = this.last_start_time + this.last_duration - now;
            this.last_start_time = this.last_duration + this.last_start_time;
            this.last_duration = audio_buffer.duration * 1000;
        }
        this.frame_duration = (audio_buffer.duration * 1000) / (jframe_count + 1);
        source.start((future_time - 8) < 0 ? 0 : (future_time - 8) / 1e3);
        return 0;
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
        this._render();
        return;
    };
    this.seek = function (time) {
        Module._play_or_seek(this.c_player, time);
    };
    this.stop = function () {
        Module._destroy_player(this.c_player);
    };
}

// otter_player
var otter_player = {
    create_player: function (width, height, canvasElem) {
        var c_player = Module._create_player(width, height);
        if (!c_player) {
            return null;
        }
        var player = new _player(c_player);
        player.init(width, height, canvasElem);
        return player;
    }
};