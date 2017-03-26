#include "contactapp.h"
#include "contactappwin.h"

#pragma GCC diagnostic push
#pragma clang diagnostic ignored "-Wdisabled-macro-expansion"

///  \brief Contact class
struct _ContactApp {
    GtkApplication parent;  ///< Parent
};

G_DEFINE_TYPE(ContactApp, contact_app, GTK_TYPE_APPLICATION)

static void contact_app_init(ContactApp *app) { (void)app; }

static void contact_app_activate(GApplication *app) {
    ContactAppWindow *win;

    win = contact_app_window_new(CONTACT_APP(app));
    gtk_window_present(GTK_WINDOW(win));
}

static void contact_app_open(GApplication *app, GFile **files, gint n_files,
                             const gchar *hint) {
    (void)hint;
    GList *windows;
    ContactAppWindow *win;
    int i;

    windows = gtk_application_get_windows(GTK_APPLICATION(app));
    if (windows)
        win = CONTACT_APP_WINDOW(windows->data);
    else
        win = contact_app_window_new(CONTACT_APP(app));

    for (i = 0; i < n_files; i++) contact_app_window_open(win, files[i]);

    gtk_window_present(GTK_WINDOW(win));
}

static void contact_app_class_init(ContactAppClass *class) {
    G_APPLICATION_CLASS(class)->activate = contact_app_activate;
    G_APPLICATION_CLASS(class)->open = contact_app_open;
}

ContactApp *contact_app_new(void) {
    return g_object_new(CONTACT_APP_TYPE, "application-id",
                        "org.gtk.exampleapp", "flags",
                        G_APPLICATION_HANDLES_OPEN, NULL);
}

#pragma GCC diagnostic pop
