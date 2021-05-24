#ifndef __TFE_CSS_H__
#define __TFE_CSS_H__

void
set_css_for_context (GtkStyleContext *context, char *css);

void
set_css_for_display (GtkWindow *win, char *css);

void
set_font_for_display (GtkWindow *win, const char *fontfamily, const char *fontstyle, const char *fontweight, int size);

void
set_font_for_display_with_pango_font_desc (GtkWindow *win, PangoFontDescription *pango_font_desc);

#endif /* __TFE_CSS_H__ */

