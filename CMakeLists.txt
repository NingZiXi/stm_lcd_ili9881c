cmake_minimum_required(VERSION 3.15)
project(stm_lcd_ili9881c C)
add_library(stm_lcd_ili9881c src/stm_lcd_ili9881c.c)
target_include_directories(stm_lcd_ili9881c PUBLIC include)
