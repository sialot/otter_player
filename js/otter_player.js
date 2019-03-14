
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

var otter_player = {
    create_player: function (width, height) {
        var c_player = Module._create_player(width, height);

        if (!c_player) {
            return null;
        }

        return  new _player(c_player);
    }
};
