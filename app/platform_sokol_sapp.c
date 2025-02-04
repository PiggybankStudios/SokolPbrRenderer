/*
File:   platform_sokol_sapp.c
Author: Taylor Robbins
Date:   02\02\2025
Description: 
	** Holds functions that help the Platform layer create and manage a window using sokol_sapp.h
*/

void SokolLogCallback(const char* tag, u32 logLevel, u32 logId, const char* message, u32 lineNum, const char* filePath, void* userData)
{
	UNUSED(tag); //TODO: Should we output the tag?
	UNUSED(logId); //TODO: Should we output the logId?
	UNUSED(userData);
	DbgLevel dbgLevel;
	switch (logLevel)
	{
		case 0: dbgLevel = DbgLevel_Error; break;
		case 1: dbgLevel = DbgLevel_Error; break;
		case 2: dbgLevel = DbgLevel_Warning; break;
		case 3: dbgLevel = DbgLevel_Other; break;
		default: dbgLevel = DbgLevel_None; break;
	}
	DebugOutputRouter(filePath, lineNum, __func__, dbgLevel, true, message);
	if (logLevel == 0) { Assert(false); }
}

sg_environment CreateSokolEnvironment()
{
	sg_environment result = ZEROED;
	result.defaults.color_format = (sg_pixel_format)sapp_color_format();
	result.defaults.depth_format = (sg_pixel_format)sapp_depth_format();
	result.defaults.sample_count = sapp_sample_count();
	result.metal.device = sapp_metal_get_device();
	result.d3d11.device = sapp_d3d11_get_device();
	result.d3d11.device_context = sapp_d3d11_get_device_context();
	result.wgpu.device = sapp_wgpu_get_device();
	return result;
}

// +==============================+
// |    Plat_GetSokolSwapchain    |
// +==============================+
GET_SOKOL_SWAPCHAIN_DEF(Plat_GetSokolSwapchain)
{
	sg_swapchain result = ZEROED;
	result.width = sapp_width();
	result.height = sapp_height();
	result.sample_count = sapp_sample_count();
	result.color_format = (sg_pixel_format)sapp_color_format();
	result.depth_format = (sg_pixel_format)sapp_depth_format();
	result.metal.current_drawable = sapp_metal_get_current_drawable();
	result.metal.depth_stencil_texture = sapp_metal_get_depth_stencil_texture();
	result.metal.msaa_color_texture = sapp_metal_get_msaa_color_texture();
	result.d3d11.render_view = sapp_d3d11_get_render_view();
	result.d3d11.resolve_view = sapp_d3d11_get_resolve_view();
	result.d3d11.depth_stencil_view = sapp_d3d11_get_depth_stencil_view();
	result.wgpu.render_view = sapp_wgpu_get_render_view();
	result.wgpu.resolve_view = sapp_wgpu_get_resolve_view();
	result.wgpu.depth_stencil_view = sapp_wgpu_get_depth_stencil_view();
	result.gl.framebuffer = sapp_gl_get_framebuffer();
	return result;
}

// +==============================+
// |     Plat_SetMouseLocked      |
// +==============================+
// void Plat_SetMouseLocked(bool isMouseLocked)
SET_MOUSE_LOCKED_DEF(Plat_SetMouseLocked)
{
	NotNull(platformData);
	NotNull(platformData->oldAppInput);
	NotNull(platformData->currentAppInput);
	if (platformData->currentAppInput->mouse.isLocked != isMouseLocked)
	{
		sapp_lock_mouse(isMouseLocked);
		//Change the value in both old and current AppInput so the application immediately sees the value change in the AppInput it has a handle to
		platformData->oldAppInput->mouse.isLocked = isMouseLocked;
		platformData->currentAppInput->mouse.isLocked = isMouseLocked;
	}
}
