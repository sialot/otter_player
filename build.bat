emcc  -I ./include aac_decoder.c block_queue.c byte_list.c decoder_master.c h264_decoder.c hash_map.c player.c priority_queue.c ts_demuxer.c ts_loader.c --post-js js/otter_player.js --js-library ts_loader.js ./lib/libavcodec.a ./lib/libavutil.a ./lib/libswscale.a -s TOTAL_MEMORY=1048576000 -s TOTAL_STACK=10485760 -s USE_PTHREADS -s PTHREAD_POOL_SIZE=8 -s EXTRA_EXPORTED_RUNTIME_METHODS=['ccall'] -Werror -s ASSERTIONS=2  -o out/player.js 