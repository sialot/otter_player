mergeInto(LibraryManager.library, {
    _js_xhr_get_file_size: function (loadPtr, url) {
        //console.log("_js_xhr_get_file_size in!");
        return JS_XHRGetFileSize(loadPtr, UTF8ToString(url));
    },
    _js_xhr_load_file: function (loadPtr, url, start, end) {
        return JS_XHRGetFile(loadPtr, UTF8ToString(url), UTF8ToString(start), UTF8ToString(end));
    },
})
