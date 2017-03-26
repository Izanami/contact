#include "gtk.h"

#include "contactapp.h"
#include "contactappwin.h"

#pragma GCC diagnostic push
#pragma clang diagnostic ignored "-Wdisabled-macro-expansion"

/// \brief Window class
struct _ContactAppWindow {
    GtkApplicationWindow parent;  ///< Parent
};

G_DEFINE_TYPE(ContactAppWindow, contact_app_window, GTK_TYPE_APPLICATION_WINDOW)

static void contact_app_window_init(ContactAppWindow *win) {
    gtk_widget_init_template(GTK_WIDGET(win));
}

static void contact_app_window_class_init(ContactAppWindowClass *class) {
    gtk_widget_class_set_template_from_resource(
        GTK_WIDGET_CLASS(class), "/org/gtk/contactapp/window.ui");
}

ContactAppWindow *contact_app_window_new(ContactApp *app) {
    return g_object_new(CONTACT_APP_WINDOW_TYPE, "application", app, NULL);
}

void contact_app_window_open(ContactAppWindow *win, GFile *file) {
    (void)win;
    (void)file;
}

#pragma GCC diagnostic pop
