#include "gtk.h"

#include "contactapp.h"
#include "contactappwin.h"

#pragma GCC diagnostic push
#pragma clang diagnostic ignored "-Wdisabled-macro-expansion"

struct _ContactAppWindow {
    GtkApplicationWindow parent;
};

G_DEFINE_TYPE(ContactAppWindow, contact_app_window, GTK_TYPE_APPLICATION_WINDOW)

static void contact_app_window_init(ContactAppWindow *app) { (void)app; }

static void contact_app_window_class_init(ContactAppWindowClass *class) {
    (void)class;
}

ContactAppWindow *contact_app_window_new(ContactApp *app) {
    return g_object_new(CONTACT_APP_WINDOW_TYPE, "application", app, NULL);
}

void contact_app_window_open(ContactAppWindow *win, GFile *file) {
    (void)win;
    (void)file;
}

#pragma GCC diagnostic pop
