#include <obs-module.h>
#include <pango/pangocairo.h>

enum {
	FORMAT_MAXLEN = 64,
	FONT_MAXLEN = 64,
	TEXT_MAXLEN = 64,
	PERIOD_S = 1,
};

struct time_source {
	float elapsed;
	uint32_t width, height;
	gs_texture_t *tex;

	struct vec4 bg, fg, outline;
	double outlinew;
	char format[FORMAT_MAXLEN + 1];
	char font[FONT_MAXLEN + 1];
};

static const char *
time_source_get_name(void *type_data)
{
	UNUSED_PARAMETER(type_data);
	return obs_module_text("TimeSource");
}

static void
render(struct time_source *context)
{
	/* Format time */
	time_t t = time(NULL);
	struct tm *tmp = localtime(&t);
	char buf[TEXT_MAXLEN + 1];
	size_t sz = strftime(buf, TEXT_MAXLEN + 1, context->format, tmp);
	if (!sz) {
		buf[0] = '\0';
	}

	/* Layout */
	PangoContext *pango = pango_font_map_create_context(
		pango_cairo_font_map_get_default());
	PangoLayout *layout = pango_layout_new(pango);
	PangoFontDescription *desc = pango_font_description_from_string(
		context->font);
	pango_layout_set_font_description(layout, desc);
	pango_font_description_free(desc);
	pango_layout_set_text(layout, buf, TEXT_MAXLEN);
	int width, height;
	pango_layout_get_pixel_size(layout, &width, &height);
	assert(width >= 0 && height >= 0);
	context->width = width;
	context->height = height;

	/* Render */
	cairo_surface_t *surface = cairo_image_surface_create(
		CAIRO_FORMAT_ARGB32, context->width, context->height);
	cairo_t *cr = cairo_create(surface);
	cairo_set_source_rgba(cr, context->bg.x, context->bg.y, context->bg.z,
		context->bg.w);
	cairo_paint(cr);
	cairo_set_source_rgba(cr, context->outline.x, context->outline.y,
		context->outline.z, context->outline.w);
	cairo_set_line_width(cr, context->outlinew);
	pango_cairo_layout_path(cr, layout);
	cairo_stroke(cr);
	cairo_set_source_rgba(cr, context->fg.x, context->fg.y, context->fg.z,
		context->fg.w);
	pango_cairo_show_layout(cr, layout);

	/* OBS texture */
	cairo_surface_flush(surface);
	unsigned char *textdata = cairo_image_surface_get_data(surface);
	obs_enter_graphics();
	if (context->tex) {
		gs_texture_destroy(context->tex);
	}
	context->tex = gs_texture_create(context->width, context->height,
		GS_BGRA, 1, (const uint8_t **)&textdata, 0);
	obs_leave_graphics();

	/* Cleanup */
	cairo_destroy(cr);
	cairo_surface_destroy(surface);
	g_object_unref(layout);
	g_object_unref(pango);
}

static void
time_source_update(void *data, obs_data_t *settings)
{
	struct time_source *context = data;

	strncpy(context->format, obs_data_get_string(settings, "format"),
		FORMAT_MAXLEN);
	obs_data_t *font_obj = obs_data_get_obj(settings, "font");
	snprintf(context->font, FONT_MAXLEN, "%s %lld",
		obs_data_get_string(font_obj, "face"),
		obs_data_get_int(font_obj, "size"));
	obs_data_release(font_obj);
	vec4_from_rgba(&context->bg, obs_data_get_int(settings, "bg"));
	vec4_from_rgba(&context->fg, obs_data_get_int(settings, "fg"));
	vec4_from_rgba(&context->outline, obs_data_get_int(settings, "outline"));
	context->outlinew = obs_data_get_double(settings, "outlinew");

	render(context);
}

static void *
time_source_create(obs_data_t *settings, obs_source_t *source)
{
	UNUSED_PARAMETER(source);

	struct time_source *context = bzalloc(sizeof(struct time_source));
	time_source_update(context, settings);
	render(context);

	return context;
}

static void
time_source_destroy(void *data)
{
	struct time_source *context = data;

	if (context->tex) {
		obs_enter_graphics();
		gs_texture_destroy(context->tex);
		obs_leave_graphics();
	}

	bfree(context);
}

static uint32_t
time_source_getwidth(void *data)
{
	struct time_source *context = data;
	return context->width;
}

static uint32_t
time_source_getheight(void *data)
{
	struct time_source *context = data;
	return context->height;
}

static void
time_source_tick(void *data, float seconds)
{
	struct time_source *context = data;
	context->elapsed += seconds;
}

static void
time_source_render(void *data, gs_effect_t *effect)
{
	UNUSED_PARAMETER(effect);

	struct time_source *context = data;

	if (context->elapsed >= PERIOD_S) {
		render(context);
		context->elapsed = 0;
	}

	obs_source_draw(context->tex, 0, 0, context->width, context->height,
		false);
}

static void
time_source_defaults(obs_data_t *settings)
{
	obs_data_set_default_string(settings, "format", "%Y-%m-%d %H:%M:%S");
	obs_data_t *font_obj = obs_data_create();
	obs_data_set_default_string(font_obj, "face", "Sans Serif");
	obs_data_set_default_int(font_obj, "size", 72);
	obs_data_set_default_obj(settings, "font", font_obj);
	obs_data_release(font_obj);
	obs_data_set_default_int(settings, "bg", 0x00000000);
	obs_data_set_default_int(settings, "fg", 0xFFFFFFFF);
	obs_data_set_default_int(settings, "outline", 0xFF000000);
	obs_data_set_default_double(settings, "outlinew", 10.0);
}

static obs_properties_t *
time_source_properties(void *data)
{
	UNUSED_PARAMETER(data);

	obs_properties_t *props = obs_properties_create();
	obs_properties_add_text(props, "format",
		obs_module_text("TimeSource.Format"), OBS_TEXT_DEFAULT);
	obs_property_set_long_description(obs_properties_get(props, "format"),
		obs_module_text("TimeSource.FormatDesc"));
	obs_properties_add_font(props, "font",
		obs_module_text("TimeSource.Font"));
	obs_properties_add_color_alpha(props, "bg",
		obs_module_text("TimeSource.BackgroundColor"));
	obs_properties_add_color_alpha(props, "fg",
		obs_module_text("TimeSource.ForegroundColor"));
	obs_properties_add_color_alpha(props, "outline",
		obs_module_text("TimeSource.OutlineColor"));
	obs_properties_add_float(props, "outlinew",
		obs_module_text("TimeSource.OutlineWidth"), 0.0, INFINITY, 1.0);

	return props;
}

struct obs_source_info time_source_info = {
	.id = "time_source",
	.type = OBS_SOURCE_TYPE_INPUT,
	.output_flags = OBS_SOURCE_VIDEO,
	.get_name = time_source_get_name,
	.create = time_source_create,
	.destroy = time_source_destroy,
	.get_width = time_source_getwidth,
	.get_height = time_source_getheight,
	.video_tick = time_source_tick,
	.video_render = time_source_render,
	.get_defaults = time_source_defaults,
	.get_properties = time_source_properties,
	.update = time_source_update,
	.icon_type = OBS_ICON_TYPE_TEXT,
};

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE("time-source", "en-US")

bool
obs_module_load(void)
{
	obs_register_source(&time_source_info);
	return true;
}
