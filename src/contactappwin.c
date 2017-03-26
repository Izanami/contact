#include "gtk.h"

#include "contactapp.h"
#include "contactappwin.h"
#include "tree.h"

#pragma GCC diagnostic push
#pragma clang diagnostic ignored "-Wdisabled-macro-expansion"
#pragma clang diagnostic ignored "-Wused-but-marked-unused"

/// \brief Window class
struct _ContactAppWindow {
    GtkApplicationWindow parent;  ///< Parent
};

/// \brief Private attribute
struct _ContactAppWindowPrivate {
    GtkWidget *stack;
};

typedef struct _ContactAppWindowPrivate ContactAppWindowPrivate;

G_DEFINE_TYPE_WITH_PRIVATE(ContactAppWindow, contact_app_window,
                           GTK_TYPE_APPLICATION_WINDOW)

static void contact_app_window_init(ContactAppWindow *app) {
    gtk_widget_init_template(GTK_WIDGET(app));
    contact_tree_new();
}

static void contact_app_window_class_init(ContactAppWindowClass *class) {
    gtk_widget_class_set_template_from_resource(
        GTK_WIDGET_CLASS(class), "/org/gtk/contactapp/window.ui");
    gtk_widget_class_bind_template_child_private(GTK_WIDGET_CLASS(class),
                                                 ContactAppWindow, stack);
}

ContactAppWindow *contact_app_window_new(ContactApp *app) {
    return g_object_new(CONTACT_APP_WINDOW_TYPE, "application", app, NULL);
}

void contact_app_window_open(ContactAppWindow *win, GFile *file) {
    ContactAppWindowPrivate *priv;

    /* Stack */
    gchar *basename;
    basename = g_file_get_basename(file);
    priv = contact_app_window_get_instance_private(win);

    /* Scroll */
    GtkWidget *scrolled;
    scrolled = gtk_scrolled_window_new(NULL, NULL);
    gtk_widget_show(scrolled);
    gtk_widget_set_hexpand(scrolled, TRUE);
    gtk_widget_set_vexpand(scrolled, TRUE);
    gtk_stack_add_titled(GTK_STACK(priv->stack), scrolled, basename, basename);

    /* List */
    ContactTree *store = contact_tree_new();
    contact_tree_set_window(store, GTK_WINDOW(win));
    contact_tree_open(store, file);
    GtkWidget *tree;
    tree = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));

    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column;

    renderer = gtk_cell_renderer_text_new();

    /* Columns */
    column = gtk_tree_view_column_new_with_attributes(
        "Prénom", renderer, "text", COLUMN_FIRSTNAME, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(tree), column);
    column = gtk_tree_view_column_new_with_attributes("Nom", renderer, "text",
                                                      COLUMN_LASTNAME, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(tree), column);
    column = gtk_tree_view_column_new_with_attributes(
        "Courriel", renderer, "text", COLUMN_MAIL, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(tree), column);
    column = gtk_tree_view_column_new_with_attributes(
        "Téléphone", renderer, "text", COLUMN_PHONE, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(tree), column);
    column = gtk_tree_view_column_new_with_attributes(
        "Date de naissance", renderer, "text", COLUMN_BIRTH, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(tree), column);

    gtk_widget_show(tree);
    gtk_container_add(GTK_CONTAINER(scrolled), tree);

    g_free(basename);
}

#pragma GCC diagnostic pop
