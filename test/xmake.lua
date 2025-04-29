target("threadsafe_ringbuffer_test")
set_kind("binary")
add_tests("default")
add_includedirs("../")
add_files("threadsafe_ringbuffer_test.cpp")

target("lockfree_queue_test")
set_kind("binary")
add_tests("default")
add_includedirs("../")
add_files("lockfree_queue_test.cpp")

target("clock_reference_test")
set_kind("binary")
add_tests("default")
add_includedirs("../")
add_files("../src/clock_reference.cpp", "clock_reference_test.cpp")

target("dpl_log_test")
set_kind("binary")
add_tests("default")
add_includedirs("../")
add_files("../src/dpl_log.cpp", "dpl_log_test.cpp")

target("dpl_demuxer_test")
set_kind("binary")
add_tests("default")
add_includedirs("../")
add_files("../src/dpl_log.cpp", "../src/demuxer.cpp", "dpl_demuxer_test.cpp")
add_packages("libavformat", "libavcodec", "libavutil", "pthread")

target("dpl_decoder_test")
set_kind("binary")
add_tests("default")
add_includedirs("../")
add_files("dpl_decoder_test.cpp", "../src/demuxer.cpp", "../src/decoder.cpp", "../src/dpl_log.cpp")
add_packages("libavformat", "libavcodec", "libavutil", "pthread")

-- target("qaudio_test")
-- set_kind("binary")
-- add_includedirs("../")
-- add_files("qaudio_test.cpp") -- 你的测试代码文件
-- add_packages("qt5core", "qt5multimedia") -- Qt 核心和多媒体模块

-- target("qt_gui_test")
-- add_rules("qt.widgetapp")
-- add_includedirs("../")
-- add_files("raster_window.h", "raster_window.cpp", "qt_gui_test.cpp") -- 你的测试代码文件
-- add_packages("qt5core", "qt5gui") -- Qt 核心和多媒体模块

target("test_sdl_video")
set_kind("binary")
add_includedirs("../")
add_files("test_sdl_video.cpp")
add_packages("sdl2")

target("audio_driver_test")
set_kind("binary")
add_includedirs("../")
add_files("audio_driver_test.cpp", "../src/audio_driver_sdl.cpp", "../src/dpl_log.cpp")
add_packages("sdl2")

-- target("audio_render_test")
--     set_kind("binary")
--     add_tests("default")
--     add_includedirs("../")
--     add_links("avcodec", "avformat", "avutil")
--     add_cxflags("-O0")
--     add_files("audio_render_test.cpp", "../src/audio_render.cpp", "../src/dpl_log.cpp")
--     add_packages("sdl2")
