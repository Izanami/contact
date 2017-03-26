#include "gtk.h"

#include "tree.h"

#pragma GCC diagnostic push
#pragma clang diagnostic ignored "-Wdisabled-macro-expansion"
#pragma clang diagnostic ignored "-Wused-but-marked-unused"

/// \brief Window class
struct _ContactTree {
    GtkListStore parent;  ///< Parent
};

struct _ContactTreePrivate {
    GtkWindow *win;
};

typedef struct _ContactTreePrivate ContactTreePrivate;

G_DEFINE_TYPE_WITH_PRIVATE(ContactTree, contact_tree, GTK_TYPE_LIST_STORE)

static void contact_tree_init(ContactTree *tree) {
    ContactTreePrivate *priv;
    priv = contact_tree_get_instance_private(tree);
    priv->win = NULL;

    GtkListStore *list = GTK_LIST_STORE(tree);
    GType column[] = {G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,
                      G_TYPE_STRING, G_TYPE_STRING};

    gtk_list_store_set_column_types(list, N_COLUMNS, column);

    GtkTreeIter iter;
    gtk_list_store_append(list, &iter);
}

static void contact_tree_class_init(ContactTreeClass *class) { (void)class; }

ContactTree *contact_tree_new() {
    return g_object_new(CONTACT_TREE_TYPE, NULL, NULL, NULL);
}

gboolean contact_tree_open(ContactTree *tree, GFile *file) {
    ContactTreePrivate *priv;
    priv = contact_tree_get_instance_private(tree);

    (void)tree;

    GError *err = NULL;
    GFileInputStream *stream = g_file_read(file, NULL, &err);

    // If error, then display a dialog
    if (err != NULL) {
        GtkDialogFlags flags = GTK_DIALOG_DESTROY_WITH_PARENT;
        GtkWidget *dialog =
            gtk_message_dialog_new(priv->win, flags, GTK_MESSAGE_ERROR,
                                   GTK_BUTTONS_CLOSE, "%s", err->message);
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        return FALSE;
    }

    GDataInputStream *data = g_data_input_stream_new(G_INPUT_STREAM(stream));

    gsize length;
    char *line;

    do {
        line = g_data_input_stream_read_line_utf8(data, &length, NULL, &err);
        g_free(line);
    } while (line != NULL);

    return TRUE;
}

gboolean contact_tree_set_window(ContactTree *tree, GtkWindow *win) {
    ContactTreePrivate *priv;
    priv = contact_tree_get_instance_private(tree);
    priv->win = win;
    return TRUE;
}

#pragma GCC diagnostic pop
