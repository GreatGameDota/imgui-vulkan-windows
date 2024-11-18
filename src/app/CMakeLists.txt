
add_executable(
 vulkan
 main.cpp
 gui/Application.cpp
 graphics/VulkanWindow.cpp
 ../${IMGUI_PATH}/imgui.cpp 
 ../${IMGUI_PATH}/imgui_demo.cpp
 ../${IMGUI_PATH}/imgui_draw.cpp 
 ../${IMGUI_PATH}/imgui_tables.cpp
 ../${IMGUI_PATH}/imgui_widgets.cpp
 ../${IMGUI_PATH}/backends/imgui_impl_glfw.cpp
 ../${IMGUI_PATH}/backends/imgui_impl_vulkan.cpp 
 ../${IMPLOT_PATH}/implot.cpp 
 ../${IMPLOT_PATH}/implot_items.cpp 
 ../${IMPLOT_PATH}/implot_demo.cpp 
)

target_include_directories(
 vulkan PUBLIC
 ../include
 ../include/gui
 ../include/graphics
 ../lib/imgui
 ../lib/imgui/backends
 ../lib/implot
 ../lib/glfw/include
 ../lib/glm
 ../lib/stb_image
 ${Vulkan_INCLUDE_DIRS}
 ${Boost_INCLUDE_DIRS}
)

target_link_libraries(
 vulkan
 ImGui
 ImPlot
 glfw
 ${Vulkan_LIBRARIES}
 ${Boost_LIBRARIES}
)

target_compile_definitions(vulkan PUBLIC -DImTextureID=ImU64)

target_link_options(vulkan PRIVATE
  $<$<CXX_COMPILER_ID:MSVC>:/ENTRY:mainCRTStartup>
)

# No console window for non-debug builds
set_target_properties(vulkan PROPERTIES WIN32_EXECUTABLE
  $<AND:$<PLATFORM_ID:Windows>,$<NOT:$<CONFIG:Debug,RelWithDebInfo>>>
)

add_subdirectory(../lib/glfw binary_dir EXCLUDE_FROM_ALL)