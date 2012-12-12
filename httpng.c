#include <gtk/gtk.h>
#include <cairo/cairo.h>
#include <webkit/webkit.h>

static gboolean load_images = FALSE;
static gboolean show_favicon = FALSE;
static gint device_width = 800;
static gint device_height = 600;
static gint width = 800;
static gint height = 600;
static gint zoom = 100;
static gint x = 0;
static gint y = 0;
static gchar* user_agent;
static gchar** rest = NULL;

static gboolean is_page_loaded = FALSE;
static gboolean is_icon_loaded = FALSE;

static GOptionEntry entries[] = {
  { "images", 'i', 0, G_OPTION_ARG_NONE, &load_images, "Load images", NULL },
  { "favicon", 'f', 0, G_OPTION_ARG_NONE, &show_favicon, "Add favicon", NULL },
  { "width", 'w', 0, G_OPTION_ARG_INT, &width, "Image width (Default: 800)", NULL },
  { "height", 'h', 0, G_OPTION_ARG_INT, &height, "Image height (Default: 600)", NULL },
  { "dwidth", 0, 0, G_OPTION_ARG_INT, &device_width, "Device width (Default: 800)", NULL },
  { "dheight", 0, 0, G_OPTION_ARG_INT, &device_height, "Device height (Default: 600)", NULL },
  { "zoom", 'z', 0, G_OPTION_ARG_INT, &zoom, "Zoom level (25-500)% (Default: 100)", NULL },
  { "xpan", 'x', 0, G_OPTION_ARG_INT, &x, "Horizontal panning (Default: 0)", NULL },
  { "ypan", 'y', 0, G_OPTION_ARG_INT, &y, "Veritical panning (Default: 0)", NULL },
  { "user-agent", 'u', 0, G_OPTION_ARG_STRING, &user_agent, "User agent string", NULL },
  { G_OPTION_REMAINING, 0, 0, G_OPTION_ARG_STRING_ARRAY, &rest, NULL, NULL },

  { NULL }
};

void page_loaded(GtkWidget* view, gpointer data);
void icon_loaded(GtkWidget* view, gpointer data);
gboolean timeout(gpointer data);

int main(int argc, char** argv) {

  gtk_init(&argc, &argv);

  GError* error = NULL;
  GOptionContext* context;

  context = g_option_context_new("url filename.png");
  g_option_context_set_summary(context, "Take a thumbnail of a website.");
  g_option_context_add_main_entries(context, entries, NULL);
  g_option_context_add_group(context, gtk_get_option_group(TRUE));
  if (!g_option_context_parse(context, &argc, &argv, &error)) {
    g_printerr("option parsing failed: %s\n", error->message);
    return 1;
  }

  if (!rest || !rest[1]) {
    g_printerr("%s", g_option_context_get_help(context, TRUE, NULL)); 
    return 1;
  }

  //GtkWidget* window = gtk_offscreen_window_new();
  GtkWidget* window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

  g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

  gtk_window_set_default_size(GTK_WINDOW(window), device_width, device_height);
  gtk_window_set_title(GTK_WINDOW(window), "httpng");

  GtkWidget* view = webkit_web_view_new();
  webkit_web_view_set_zoom_level(WEBKIT_WEB_VIEW(view), zoom/100.0f);
  WebKitWebSettings* settings = webkit_web_view_get_settings(WEBKIT_WEB_VIEW(view));
  g_object_set(G_OBJECT(settings), "enable-scripts", TRUE, NULL);
  g_object_set(G_OBJECT(settings), "enable-html5-database", FALSE, NULL);
  g_object_set(G_OBJECT(settings), "enable-html5-local-storage", FALSE, NULL);
  g_object_set(G_OBJECT(settings), "enable-offline-web-application-cache", FALSE, NULL);
  g_object_set(G_OBJECT(settings), "enable-plugins", FALSE, NULL);
  g_object_set(G_OBJECT(settings), "media-playback-allows-inline", FALSE, NULL);
  g_object_set(G_OBJECT(settings), "media-playback-requires-user-gesture", TRUE, NULL);
  
  g_object_set(G_OBJECT(settings), "user-agent", user_agent, NULL);
  g_object_set(G_OBJECT(settings), "auto-load-images", load_images, NULL);

  GtkWidget* scrollbars = gtk_scrolled_window_new(NULL, NULL);
  //gtk_container_add(GTK_CONTAINER(scrollbars), view);
  gtk_container_add(GTK_CONTAINER(window), view);

  g_signal_connect(view, "notify::load-status", G_CALLBACK(page_loaded), NULL);
  webkit_web_view_load_uri(WEBKIT_WEB_VIEW(view), rest[0]);

  g_signal_connect(view, "icon-loaded", G_CALLBACK(icon_loaded), NULL);

  g_timeout_add_seconds(20, timeout, view);

  gtk_widget_show_all(window);

  gtk_main();

  return 0;
}

void snap(GtkWidget* view, gpointer data) {

  GdkPixbuf* favicon = webkit_web_view_try_get_favicon_pixbuf(WEBKIT_WEB_VIEW(view), 16, 16);

  GtkAllocation allocation;
  gtk_widget_get_allocation(view, &allocation);

  cairo_surface_t* surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
  
  cairo_t* cr = cairo_create(surface);

  cairo_save(cr);

  gdouble scalex = (gdouble) width / (gdouble) allocation.width;
  gdouble scaley = (gdouble) height / (gdouble) allocation.height;

  if (scaley > scalex*2.0)
    cairo_scale(cr, scalex, scalex);
  else
    cairo_scale(cr, scalex, scaley);

  // Draw main window
  WebKitWebFrame* frame = webkit_web_view_get_main_frame(WEBKIT_WEB_VIEW(view));

  cairo_set_antialias(cr, CAIRO_ANTIALIAS_SUBPIXEL);
  gtk_widget_draw(view, cr);
  cairo_restore(cr);

  // Try adding favicon
  if (favicon && show_favicon) {
    gdk_cairo_set_source_pixbuf(cr, favicon, 8, height - gdk_pixbuf_get_height(favicon) - 8);
    cairo_paint(cr);
  }

  cairo_surface_write_to_png(surface, rest[1]);

  cairo_destroy(cr);
  cairo_surface_destroy(surface);

  gtk_main_quit();
}

gboolean timeout(gpointer data) {

  GtkWidget* view = GTK_WIDGET(data);
  snap(view, NULL);

  return false;
}

void page_loaded(GtkWidget* view, gpointer data) {

  WebKitLoadStatus status = webkit_web_view_get_load_status(WEBKIT_WEB_VIEW(view));
  
  if (status != WEBKIT_LOAD_FINISHED) {
    return;
  }

  is_page_loaded = TRUE;

  if (show_favicon) {
    if (!is_icon_loaded) {
      return;
    } else {
      snap(view, data);
    }
  } else {
    snap(view, data);
  }
}

void icon_loaded(GtkWidget* view, gpointer data) {
  
  is_icon_loaded = TRUE;

  // The page had better been loaded by this point
  if (is_page_loaded)
    snap(view, data);
}