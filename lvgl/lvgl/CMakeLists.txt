SET(SOURCES
    src/draw/lv_img_cache.c
    src/draw/lv_draw_rect.c
    src/draw/lv_img_buf.c
    src/draw/lv_draw_triangle.c
    src/draw/lv_draw_mask.c
    src/draw/lv_draw_img.c
    src/draw/lv_draw_blend.c
    src/draw/lv_draw_line.c
    src/draw/lv_draw_label.c
    src/draw/lv_img_decoder.c
    src/draw/lv_draw_arc.c
    src/widgets/lv_label.c
    src/widgets/lv_canvas.c
    src/widgets/lv_arc.c
    src/widgets/lv_checkbox.c
    src/widgets/lv_bar.c
    src/widgets/lv_roller.c
    src/widgets/lv_btn.c
    src/widgets/lv_img.c
    src/widgets/lv_objx_templ.c
    src/widgets/lv_table.c
    src/widgets/lv_btnmatrix.c
    src/widgets/lv_line.c
    src/widgets/lv_dropdown.c
    src/widgets/lv_slider.c
    src/widgets/lv_textarea.c
    src/widgets/lv_switch.c
    src/hal/lv_hal_tick.c
    src/hal/lv_hal_indev.c
    src/hal/lv_hal_disp.c
    src/core/lv_refr.c
    src/core/lv_indev.c
    src/core/lv_theme.c
    src/core/lv_obj_scroll.c
    src/core/lv_disp.c
    src/core/lv_obj.c
    src/core/lv_obj_style.c
    src/core/lv_event.c
    src/core/lv_obj_class.c
    src/core/lv_obj_draw.c
    src/core/lv_group.c
    src/core/lv_obj_style_gen.c
    src/core/lv_obj_tree.c
    src/core/lv_indev_scroll.c
    src/core/lv_obj_pos.c
    src/misc/lv_gc.c
    src/misc/lv_txt.c
    src/misc/lv_tlsf.c
    src/misc/lv_fs.c
    src/misc/lv_log.c
    src/misc/lv_mem.c
    src/misc/lv_color.c
    src/misc/lv_anim.c
    src/misc/lv_printf.c
    src/misc/lv_style_gen.c
    src/misc/lv_async.c
    src/misc/lv_area.c
    src/misc/lv_anim_timeline.c
    src/misc/lv_math.c
    src/misc/lv_ll.c
    src/misc/lv_timer.c
    src/misc/lv_txt_ap.c
    src/misc/lv_bidi.c
    src/misc/lv_style.c
    src/misc/lv_templ.c
    src/misc/lv_utils.c
    src/font/lv_font_montserrat_26.c
    src/font/lv_font_montserrat_22.c
    src/font/lv_font_loader.c
    src/font/lv_font_montserrat_20.c
    src/font/lv_font_simsun_16_cjk.c
    src/font/lv_font_montserrat_28.c
    src/font/lv_font_montserrat_34.c
    src/font/lv_font_montserrat_30.c
    src/font/lv_font_montserrat_36.c
    src/font/lv_font_montserrat_16.c
    src/font/lv_font_montserrat_12.c
    src/font/lv_font.c
    src/font/lv_font_montserrat_28_compressed.c
    src/font/lv_font_montserrat_46.c
    src/font/lv_font_montserrat_48.c
    src/font/lv_font_montserrat_44.c
    src/font/lv_font_montserrat_32.c
    src/font/lv_font_montserrat_8.c
    src/font/lv_font_montserrat_38.c
    src/font/lv_font_unscii_8.c
    src/font/lv_font_dejavu_16_persian_hebrew.c
    src/font/lv_font_montserrat_24.c
    src/font/lv_font_montserrat_10.c
    src/font/lv_font_unscii_16.c
    src/font/lv_font_montserrat_12_subpx.c
    src/font/lv_font_montserrat_18.c
    src/font/lv_font_montserrat_14.c
    src/font/lv_font_fmt_txt.c
    src/font/lv_font_montserrat_42.c
    src/font/lv_font_montserrat_40.c
    src/gpu/da1470x/lv_gpu_d2_draw_blend.c
    src/gpu/da1470x/lv_gpu_d2_draw_label.c
    src/gpu/da1470x/lv_gpu_d2_draw_img.c
    src/gpu/da1470x/lv_gpu_d2_draw_rect.c
    src/gpu/sdl/lv_gpu_sdl_stack_blur.c
    src/gpu/sdl/lv_gpu_sdl_draw_label.c
    src/gpu/sdl/lv_gpu_sdl_draw_arc.c
    src/gpu/sdl/lv_gpu_sdl_lru.c
    src/gpu/sdl/lv_gpu_sdl_draw_line.c
    src/gpu/sdl/lv_gpu_sdl_mask.c
    src/gpu/sdl/lv_gpu_sdl_draw_blend.c
    src/gpu/sdl/lv_gpu_sdl_draw_rect.c
    src/gpu/sdl/lv_gpu_sdl_texture_cache.c
    src/gpu/sdl/lv_gpu_sdl_utils.c
    src/gpu/sdl/lv_gpu_sdl_draw_img.c
    src/extra/libs/qrcode/lv_qrcode.c
    src/extra/libs/freetype/lv_freetype.c
    src/extra/libs/png/lv_png.c
    src/extra/libs/gif/gifdec.c
    src/extra/libs/gif/lv_gif.c
    src/extra/libs/sjpg/lv_sjpg.c
    src/extra/libs/fsdrv/lv_fs_stdio.c
    src/extra/libs/fsdrv/lv_fs_win32.c
    src/extra/libs/fsdrv/lv_fs_posix.c
    src/extra/libs/fsdrv/lv_fs_fatfs.c
    src/extra/libs/bmp/lv_bmp.c
    src/extra/widgets/meter/lv_meter.c
    src/extra/widgets/msgbox/lv_msgbox.c
    src/extra/widgets/keyboard/lv_keyboard.c
    src/extra/widgets/win/lv_win.c
    src/extra/widgets/spinbox/lv_spinbox.c
    src/extra/widgets/calendar/lv_calendar_header_arrow.c
    src/extra/widgets/calendar/lv_calendar.c
    src/extra/widgets/calendar/lv_calendar_header_dropdown.c
    src/extra/widgets/imgbtn/lv_imgbtn.c
    src/extra/widgets/led/lv_led.c
    src/extra/widgets/tileview/lv_tileview.c
    src/extra/widgets/chart/lv_chart.c
    src/extra/widgets/spinner/lv_spinner.c
    src/extra/widgets/list/lv_list.c
    src/extra/widgets/tabview/lv_tabview.c
    src/extra/widgets/animimg/lv_animimg.c
    src/extra/widgets/colorwheel/lv_colorwheel.c
    src/extra/widgets/span/lv_span.c
    src/extra/lv_extra.c
    src/extra/others/snapshot/lv_snapshot.c
    src/extra/themes/mono/lv_theme_mono.c
    src/extra/themes/basic/lv_theme_basic.c
    src/extra/themes/default/lv_theme_default.c
    src/extra/layouts/grid/lv_grid.c
    src/extra/layouts/flex/lv_flex.c
)

SET(SOURCES
    src/draw/lv_img_cache.c
    src/draw/lv_draw_rect.c
    src/draw/lv_img_buf.c
    src/draw/lv_draw_triangle.c
    src/draw/lv_draw_mask.c
    src/draw/lv_draw_img.c
    src/draw/lv_draw_blend.c
    src/draw/lv_draw_line.c
    src/draw/lv_draw_label.c
    src/draw/lv_img_decoder.c
    src/draw/lv_draw_arc.c
    src/widgets/lv_label.c
    src/widgets/lv_canvas.c
    src/widgets/lv_arc.c
    src/widgets/lv_checkbox.c
    src/widgets/lv_bar.c
    src/widgets/lv_roller.c
    src/widgets/lv_btn.c
    src/widgets/lv_img.c
    src/widgets/lv_objx_templ.c
    src/widgets/lv_table.c
    src/widgets/lv_btnmatrix.c
    src/widgets/lv_line.c
    src/widgets/lv_dropdown.c
    src/widgets/lv_slider.c
    src/widgets/lv_textarea.c
    src/widgets/lv_switch.c
    src/hal/lv_hal_tick.c
    src/hal/lv_hal_indev.c
    src/hal/lv_hal_disp.c
    src/core/lv_refr.c
    src/core/lv_indev.c
    src/core/lv_theme.c
    src/core/lv_obj_scroll.c
    src/core/lv_disp.c
    src/core/lv_obj.c
    src/core/lv_obj_style.c
    src/core/lv_event.c
    src/core/lv_obj_class.c
    src/core/lv_obj_draw.c
    src/core/lv_group.c
    src/core/lv_obj_style_gen.c
    src/core/lv_obj_tree.c
    src/core/lv_indev_scroll.c
    src/core/lv_obj_pos.c
    src/misc/lv_gc.c
    src/misc/lv_txt.c
    src/misc/lv_tlsf.c
    src/misc/lv_fs.c
    src/misc/lv_log.c
    src/misc/lv_mem.c
    src/misc/lv_color.c
    src/misc/lv_anim.c
    src/misc/lv_printf.c
    src/misc/lv_style_gen.c
    src/misc/lv_async.c
    src/misc/lv_area.c
    src/misc/lv_anim_timeline.c
    src/misc/lv_math.c
    src/misc/lv_ll.c
    src/misc/lv_timer.c
    src/misc/lv_txt_ap.c
    src/misc/lv_bidi.c
    src/misc/lv_style.c
    src/misc/lv_templ.c
    src/misc/lv_utils.c
    src/font/lv_font_montserrat_26.c
    src/font/lv_font_montserrat_22.c
    src/font/lv_font_loader.c
    src/font/lv_font_montserrat_20.c
    src/font/lv_font_simsun_16_cjk.c
    src/font/lv_font_montserrat_28.c
    src/font/lv_font_montserrat_34.c
    src/font/lv_font_montserrat_30.c
    src/font/lv_font_montserrat_36.c
    src/font/lv_font_montserrat_16.c
    src/font/lv_font_montserrat_12.c
    src/font/lv_font.c
    src/font/lv_font_montserrat_28_compressed.c
    src/font/lv_font_montserrat_46.c
    src/font/lv_font_montserrat_48.c
    src/font/lv_font_montserrat_44.c
    src/font/lv_font_montserrat_32.c
    src/font/lv_font_montserrat_8.c
    src/font/lv_font_montserrat_38.c
    src/font/lv_font_unscii_8.c
    src/font/lv_font_dejavu_16_persian_hebrew.c
    src/font/lv_font_montserrat_24.c
    src/font/lv_font_montserrat_10.c
    src/font/lv_font_unscii_16.c
    src/font/lv_font_montserrat_12_subpx.c
    src/font/lv_font_montserrat_18.c
    src/font/lv_font_montserrat_14.c
    src/font/lv_font_fmt_txt.c
    src/font/lv_font_montserrat_42.c
    src/font/lv_font_montserrat_40.c
    src/gpu/da1470x/lv_gpu_d2_draw_blend.c
    src/gpu/da1470x/lv_gpu_d2_draw_label.c
    src/gpu/da1470x/lv_gpu_d2_draw_img.c
    src/gpu/da1470x/lv_gpu_d2_draw_rect.c
    src/gpu/sdl/lv_gpu_sdl_stack_blur.c
    src/gpu/sdl/lv_gpu_sdl_draw_label.c
    src/gpu/sdl/lv_gpu_sdl_draw_arc.c
    src/gpu/sdl/lv_gpu_sdl_lru.c
    src/gpu/sdl/lv_gpu_sdl_draw_line.c
    src/gpu/sdl/lv_gpu_sdl_mask.c
    src/gpu/sdl/lv_gpu_sdl_draw_blend.c
    src/gpu/sdl/lv_gpu_sdl_draw_rect.c
    src/gpu/sdl/lv_gpu_sdl_texture_cache.c
    src/gpu/sdl/lv_gpu_sdl_utils.c
    src/gpu/sdl/lv_gpu_sdl_draw_img.c
    src/extra/libs/qrcode/lv_qrcode.c
    src/extra/libs/freetype/lv_freetype.c
    src/extra/libs/png/lv_png.c
    src/extra/libs/gif/gifdec.c
    src/extra/libs/gif/lv_gif.c
    src/extra/libs/sjpg/lv_sjpg.c
    src/extra/libs/fsdrv/lv_fs_stdio.c
    src/extra/libs/fsdrv/lv_fs_win32.c
    src/extra/libs/fsdrv/lv_fs_posix.c
    src/extra/libs/fsdrv/lv_fs_fatfs.c
    src/extra/libs/bmp/lv_bmp.c
    src/extra/widgets/meter/lv_meter.c
    src/extra/widgets/msgbox/lv_msgbox.c
    src/extra/widgets/keyboard/lv_keyboard.c
    src/extra/widgets/win/lv_win.c
    src/extra/widgets/spinbox/lv_spinbox.c
    src/extra/widgets/calendar/lv_calendar_header_arrow.c
    src/extra/widgets/calendar/lv_calendar.c
    src/extra/widgets/calendar/lv_calendar_header_dropdown.c
    src/extra/widgets/imgbtn/lv_imgbtn.c
    src/extra/widgets/led/lv_led.c
    src/extra/widgets/tileview/lv_tileview.c
    src/extra/widgets/chart/lv_chart.c
    src/extra/widgets/spinner/lv_spinner.c
    src/extra/widgets/list/lv_list.c
    src/extra/widgets/tabview/lv_tabview.c
    src/extra/widgets/animimg/lv_animimg.c
    src/extra/widgets/colorwheel/lv_colorwheel.c
    src/extra/widgets/span/lv_span.c
    src/extra/lv_extra.c
    src/extra/others/snapshot/lv_snapshot.c
    src/extra/themes/mono/lv_theme_mono.c
    src/extra/themes/basic/lv_theme_basic.c
    src/extra/themes/default/lv_theme_default.c
    src/extra/layouts/grid/lv_grid.c
    src/extra/layouts/flex/lv_flex.c
)

SET(INCLUDES

    src/draw
    src/widgets
    src/hal
    src/core
    src
    src/misc
    src/gpu
    src/gpu/sdl
    src/extra/libs/qrcode
    src/extra/libs/freetype
    src/extra/libs/png
    src/extra/libs/gif
    src/extra/libs
    src/extra/libs/sjpg
    src/extra/libs/fsdrv
    src/extra/libs/bmp
    src/extra/widgets/meter
    src/extra/widgets/msgbox
    src/extra/widgets/keyboard
    src/extra/widgets
    src/extra/widgets/win
    src/extra/widgets/spinbox
    src/extra/widgets/calendar
    src/extra/widgets/imgbtn
    src/extra/widgets/led
    src/extra/widgets/tileview/
    src/extra/widgets/chart
    src/extra/widgets/spinner
    src/extra/widgets/list
    src/extra/widgets/tabview
    src/extra/widgets/animimg
    src/extra/widgets/colorwheel/
    src/extra/widgets/span
    src/extra
    src/extra/others
    src/extra/others/snapshot
    src/extra/themes/mono
    src/extra/themes/basic
    src/extra/themes
    src/extra/themes/default
    src/extra/layouts
    src/extra/layouts/grid
    src/extra/layouts/flex
    src
    .
)

add_library(lvgl ${SOURCES})

target_include_directories(lvgl PUBLIC ${INCLUDES})