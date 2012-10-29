#include <gtk/gtk.h>
#include <cairo/cairo.h>
#include <webkit/webkit.h>

static void snap(GtkWidget* widget, const char* filename) {

  GtkAllocation allocation;
  gtk_widget_get_allocation(widget, &allocation);

  printf("Downloaded page %ix%i ...\n", allocation.width, allocation.height);
  printf("Clamped page to %ix%i\n", allocation.width, allocation.width);
  cairo_surface_t* surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 512.0, 512.0);
  
  cairo_t* cr = cairo_create(surface);
  printf("Scaled page to %.4lg%%\n", 512.0/(double)allocation.width*100.0);
  cairo_scale(cr, 512.0/(double)allocation.width, 512.0/(double)allocation.width);

  // Draw main window
  WebKitWebFrame* frame = webkit_web_view_get_main_frame(WEBKIT_WEB_VIEW(widget));
  gtk_widget_draw(GTK_WIDGET(webkit_web_frame_get_web_view(frame)), cr);

  cairo_surface_write_to_png(surface, filename);

  cairo_destroy(cr);
  cairo_surface_destroy(surface);

  gtk_main_quit();
}

static void done_loading(GObject* object, GParamSpec* pspec, gpointer data) {
  
  WebKitWebView* view = WEBKIT_WEB_VIEW(object);
  WebKitLoadStatus status = webkit_web_view_get_load_status(view);
  if (status != WEBKIT_LOAD_FINISHED) {
    return;
  }
  printf("Finished loading page.\n");
  snap(GTK_WIDGET(view), data);
}

int main(int argc, char* argv[]) {

  gtk_init(&argc, &argv);

  if (argc < 3) {
    fprintf(stderr, "Usage:\nhttpng www.example.com example.png\n");
    return 1;
  }

  char* scheme = g_uri_parse_scheme(argv[1]);
  if (scheme == NULL || g_strcmp0(scheme, "http")) {
    fprintf(stderr, "Bad Url\n");
    fprintf(stderr, "Usage:\nhttpng www.example.com example.png\n");
    return 1;
  }

  GRegex* extension = g_regex_new(".*\\.png$", 0, 0, NULL);
  if (!g_regex_match_all(extension, argv[2], 0, NULL)) {
    fprintf(stderr, "Filename must be of type *.png\n");
    fprintf(stderr, "Usage:\nhttpng www.example.com example.png\n");
    return 1;
  }

  GtkWidget* window = gtk_offscreen_window_new();
  g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

  gtk_window_set_default_size(GTK_WINDOW(window), 512, 512);
  gtk_window_set_title(GTK_WINDOW(window), "httpng");

  GtkWidget* view = webkit_web_view_new();

  gtk_container_add(GTK_CONTAINER(window), view);

  webkit_web_view_load_uri(WEBKIT_WEB_VIEW(view), argv[1]);

  g_signal_connect(view, "notify::load-status", G_CALLBACK(done_loading), argv[2]);

  gtk_widget_show_all(window);

  gtk_main();

  return 0;
}