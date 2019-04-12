/**
 * OTTER_PLAYER
 *
 * @author sialot
 */

// TSý�������
function ts_loader(c_player) {
    this.PKT_NUM_PER_TIME = 2000; // ÿ�μ��ذ���
    this.media_file_size = 0; // ý���С
    this.current_range = 0; // ��ǰ����λ��
    this.duration = 0; // ý��ʱ��
    this.cur_time = 0; // ��ǰʱ��
    this.media_url = ""; // ý���ַ
    this.is_can_seek = false; // �Ƿ��ʱ��
    this.is_finish = false; // �Ƿ����
    this.data_buffer; // ���ݻ���
    this.c_player = c_player;
    this.init = function (media_url, duration, cur_time) {
        this.media_file_size = 0;
        this.current_range = 0;
        this.duration = duration;
        this.cur_time = cur_time;
        this.media_url = media_url;
    };

    // ��ʼ����
    this.start = function () {

        // ��ȡ�ļ���С
        var request = new XMLHttpRequest();
        request.open("GET", this.media_url, true);
        request.responseType = "arraybuffer";
        request.url = this.media_url + "?" + Math.random();
        request.setRequestHeader("Range", "bytes=0-187");
        request.onreadystatechange = function () {
            if (request.readyState == 4) {
                if (request.status == 206) {
                    var content_range = request.getResponseHeader("Content-Range");
                    if (content_range != undefined) {
                        this.media_file_size = content_range.split("/")[1];
                    }

                    // ��ǰʱ���Ѿ�����
                    if (this.cur_time > this.duration){
                        this.current_range = this.media_file_size;
                        this.is_finish = true;
                    }

                    // ��ʱ��
                    if (this.media_file_size > 0 && this.duration > 0)
                    {
                        // �������λ��
                        var wishSize = (this.cur_time / this.duration) * this.media_file_size;
                        this.current_range = Math.floor(wishSize / 188.0) * 188; 
                        this.is_can_seek = true;
                    }

                    // �ֶμ��Ӱ�
                    this._loadData();
                }
                else {
                    console.error("ts file size load failed!");
                    this.is_finish = true;
                }
            }
        }.bind(this);
        request.send();
    };

    // �ֶμ���
    this._loadData= function () {

        if (this.is_finish) {
            return;
        }

        // ���range
        let start = this.current_range;
        let end = start + this.PKT_NUM_PER_TIME * 188 - 1;
        if (end >this.media_file_size) {
            end = this.media_file_size - 1;
        }

        // ���Լ����������
        if (Module._js_can_load_file(this.c_player)) {
            var request = new XMLHttpRequest();
            var range = "bytes=" + start + "-" + end;
            //console.log("load data:" + range);
            var len = 0;
            request.open("GET", this.media_url, true);
            request.responseType = "arraybuffer";
            request.url = this.media_url + "?" + Math.random();
            request.setRequestHeader("Range", range);
            request.onreadystatechange = function () {
                if (request.readyState == 4) {
                    if (request.status == 206) {

                        var arrayBuffer = request.response;
                        if (arrayBuffer) {
                            var byteArray = new Uint8Array(arrayBuffer);
                            len = byteArray.byteLength;

                            // ��ʼ������
                            if (this.data_buffer == undefined) {
                                this.data_buffer = Module._malloc(len);
                            }

                            // Ϊ���渳ֵ
                            for (var i = 0; i < len; i++) {
                                Module.HEAP8[this.data_buffer + i] = byteArray[i];
                            }

                            // ����C��������
                            Module.ccall('js_push_data', 'null', ['number', 'number', 'number'], [this.c_player, this.data_buffer, len]);

                            // ����range
                            this.current_range += this.PKT_NUM_PER_TIME * 188;
                            if (this.media_file_size != 0 && this.current_range >= this.media_file_size) {
                                this.is_finish = true;
                            }
                            else {

                                // ѭ������
                                setTimeout(function () { this._loadData() }.bind(this), 20);
                            }
                        }
                    }
                    else {
                        console.error("ts file load failed!");
                    }
                }
            }.bind(this);
            request.send();
        } else {
            
            // ѭ������
            setTimeout(function () { this._loadData() }.bind(this), 100);
        }
    };
}

// JS player�ඨ��
function _player(c_player) {
    this.c_player = c_player;
    this.play_state = 0; // 0 ��ʼ 1���� 2��ͣ 3ֹͣ
    this.ts_loader = new ts_loader(c_player);
    this.inited = false;
    this.finish = 0;
    this.player_cur_time = 0;
    this.media_start_pts = -1; // ý���һ��pts��

    // ֡׼��
    this.retry = !1;
    this.last_try_time = 0;
    this.MERGE_COUNT = 48;

    // ��Ƶ���
    this.witdh = 0;
    this.height = 0;
    this.canvasElem;
    this.canvas_ctx;
    this.cur_video_pts;
    this.last_dis_time = 0; // �ϴ���ʾ֡��ʱ��
    this.video_frame_buffer_arr = new Array(96); // ��Ƶ֡����
    this.video_frame_count = 0; // ��Ƶ֡����

    // ��Ƶ���
    this.audio_player;
    this.audio_ctx;
    this.last_duration = 0; // ��һ֡ʱ��
    this.last_start_time = 0; // ��һ֡��ʼʱ��
    this.next_audio_pts = 0;

    // ��ʼ��
    this.init = function (width, height, canvasElem) {
        this.current_time = 0;
        let AudioContext = window.AudioContext || window.webkitAudioContext;
        this.audio_ctx = AudioContext ? new AudioContext() : '';
        this.witdh = width;
        this.height = height;
		this.cur_video_pts = 0;
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
        this.inited = true;
    };
    this._frame_time_temp = -1;
    this._getFrameTime = function () {

        if (this._frame_time_temp != -1) {
            return this._frame_time_temp;
        }

        if (this.cur_video_pts == 0) {
            return 0;
        }
        let left_real_time = this.last_start_time + this.last_duration - performance.now();
        let left_pts_time = this.next_audio_pts - this.cur_video_pts;
        let left_frame_count = left_pts_time / 40;
        this._frame_time_temp = (left_real_time / left_frame_count) - 43;
        this._frame_time_temp = this._frame_time_temp > 40 ? 40 : this._frame_time_temp;
        return this._frame_time_temp;
    };

    // ��Ⱦ
    this._render = function () {
        if (this.finish) {
            console.log("audio finish!");
            return;
        }
        var now = performance.now();
        if (this.last_start_time != 0 && (this.last_start_time + this.last_duration - now) < -5000) {
            this.finish = 1;
        }

        // ��Ƶ��Ⱦ
        if (this.last_start_time != 0) {
            if ((now - this.last_dis_time) >= this._getFrameTime()) {
                this._frame_time_temp = -1;
                var min_idx = -1;
                var min_frame_time = -1;

                // �ҵ���С��ʱ��
                for (i = 0; i < this.video_frame_buffer_arr.length; i++) {

                    // ����������
                    if (this.video_frame_buffer_arr[i] == undefined || this.video_frame_buffer_arr[i].time == -1) {
                        continue;
                    }
                  
                    let f_time = this.video_frame_buffer_arr[i].time;

                    // ������ʱ֡
                    if (this.cur_video_pts > f_time) {
                        console.log("throw:" + f_time + " this.cur_video_pts:" + this.cur_video_pts + " prt:" + this.video_frame_buffer_arr[i].dataPtr);
                        Module._free(this.video_frame_buffer_arr[i].dataPtr);
                        this.video_frame_buffer_arr[i].time = -1;
                        continue;
                    }

                    // ʱ��Ա�
                    if (min_frame_time == -1) {
                        min_frame_time = f_time;
                        min_idx = i;
                    } else {
                        if (f_time < min_frame_time) {
                            min_frame_time = f_time;
                            min_idx = i;
                        }
                    }
                }

                if (min_idx >= 0) {

                    // ֡ʱ��
                    let f_time = this.video_frame_buffer_arr[min_idx].time;
                    let imgData = this.video_frame_buffer_arr[min_idx].data;
                    let dataPtr = this.video_frame_buffer_arr[min_idx].dataPtr;
                    let j = 0, imgIdx = 0;

                    for (; imgIdx < imgData.data.length;) {
                        imgData.data[imgIdx] = Module.HEAPU8[dataPtr + j];
                        imgData.data[imgIdx + 1] = Module.HEAPU8[dataPtr + j + 1];
                        imgData.data[imgIdx + 2] = Module.HEAPU8[dataPtr + j + 2];
                        imgData.data[imgIdx + 3] = 255;
                        imgIdx = imgIdx + 4;
                        j = j + 3;
                    }
                    Module._free(dataPtr);
                    this.canvas_ctx.putImageData(imgData, 0, 0);
                    this.cur_video_pts = f_time;
                    this.last_dis_time = now;
                    this.video_frame_buffer_arr[min_idx].time = -1;
                } else {
                    console.log("no frame waiting");
                }
            }
        }
        var now = performance.now();

        // ����
        if (this.retry) {
            if ((now - this.last_try_time) > 200) {
                this.retry = !1;
                this.last_try_time = 0;
                this._prepare_source();
            }
        } else {

            // ����׼��
            if ((this.last_start_time + this.last_duration - now) <= this.last_duration / 2) {
                this._prepare_source();
            }
        }
		var id = requestAnimationFrame(function () { this._render() }.bind(this));
    },

    // ����׼��
    this._prepare_source = function () {

        var jframe_arr = new Array(this.MERGE_COUNT);
        var audio_frame_count = 0; // ���ٸ�js��Ƶ֡
        var audio_pcm_frame_count = 0; // ������pcm֡��
        var channels; // ������

        var audio_future_pts = -1;

        var _video_frame_count = 0;

        for (; audio_frame_count < this.MERGE_COUNT;) {
            var jframePtr = Module._js_poll_frame(this.c_player);

            if (jframePtr == 0) {
				console.log(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> zero");
                break;
            }

            var frame_item = new Object();

            // ���ݳ���
            frame_item.len = Module.HEAPU32[jframePtr >> 2];

            // ��ǰʱ��
            frame_item.cur_pts = Module.HEAPU32[(jframePtr >> 2) + 1];

            // ֡����
            frame_item.av_type = Module.HEAPU32[(jframePtr >> 2) + 2];

            // ��ƵƵ����
            frame_item.channels = Module.HEAP32[(jframePtr >> 2) + 3];

            // ����ָ��
            frame_item.dataPtr = Module.HEAP32[(jframePtr >> 2) + 4];

            // ��Ƶ
            if (frame_item.av_type == 1) {

                let targe_index = this.video_frame_count % this.video_frame_buffer_arr.length;
                if(this.video_frame_buffer_arr[targe_index] == undefined){
                    var f_data = this.canvas_ctx.createImageData(this.canvasElem.width, this.canvasElem.height);
                    this.video_frame_buffer_arr[targe_index] = { data: f_data, time: -1, dataPtr: 0 };
                }

                if (this.video_frame_buffer_arr[targe_index].time != -1) {
                    console.log("overwrite:" + this.video_frame_buffer_arr[targe_index].time + " this.cur_video_pts:" + this.cur_video_pts + " prt:" + this.video_frame_buffer_arr[targe_index].dataPtr);
                    Module._free(this.video_frame_buffer_arr[targe_index].dataPtr);
                    this.video_frame_buffer_arr[targe_index].time = -1;
                }

				this.video_frame_buffer_arr[targe_index].dataPtr = frame_item.dataPtr;
				this.video_frame_buffer_arr[targe_index].time = frame_item.cur_pts;
				this.video_frame_count++;

            } else {

                if (this.media_start_pts < 0) {
                    this.media_start_pts = frame_item.cur_pts;
                }

                audio_future_pts = frame_item.cur_pts;

                // ����֡��
                channels = frame_item.channels;
                audio_pcm_frame_count += frame_item.len / 4 / frame_item.channels; // ������֡��
                jframe_arr[audio_frame_count] = frame_item;
                audio_frame_count++;
            }
            Module._free(jframePtr);
        }
		
        if (audio_frame_count == 0) {
            this.retry = !0;
            this.last_try_time = performance.now();
            return;
        }

        // ��Ƶ
        var audio_buffer = this.audio_ctx.createBuffer(channels, audio_pcm_frame_count, this.audio_ctx.sampleRate);

        let idx = 0;
        for (var i = 0; i < audio_frame_count; i++) {
            let frame_item = jframe_arr[i];
            let frame_count = frame_item.len / 4 / frame_item.channels;

            // ����֡ 1-1024
            for (k = 0; k < frame_count; k++) {

                // ���� 2
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

        // ��ǰΪ��һʱ��Ƭ
        var now = performance.now();
        if (this.last_start_time == 0) {
            future_time = 0;
            this.last_start_time = now;
            this.last_duration = audio_buffer.duration * 1000;

        } else {
            future_time = this.last_start_time + this.last_duration - now;
            this.last_start_time = this.last_duration + this.last_start_time;
            this.last_duration = audio_buffer.duration * 1000;
        }
        if (audio_future_pts > 0) {
            source.onended = function () { this.cur_video_pts = audio_future_pts }.bind(this);
        }
        this.next_audio_pts = audio_future_pts;
        source.start((future_time - 8) < 0 ? 0 : (future_time - 8) / 1e3);
        return 0;
    };

    // �趨ý��
    this.set_media = function (url, duration) {
        if (!url) {
            console.error("url is undefined!");
            return;
        }
        if (!duration) {
            console.error("duration is undefined!");
            return;
        }
        this.ts_loader.init(url, duration, this.player_cur_time)
        this.ts_loader.start();
        return;
    };

    // ����
    this.play = function () {
        if (!this.inited) {
            return;
        }

        if (this.play_state == 0 || this.play_state == 2) {
            Module._play(this.c_player);
            this._render();
            this.play_state = 1;
        }

        return;
    };

    // ʱ��
    this.seek = function (time) {

        // TODO
    };
    this.stop = function () {

        // TODO
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
        setTimeout(function () { player.init(width, height, canvasElem); }, 0);        
        return player;
    }
};