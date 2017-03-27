#include "gtk.h"

#include "tree.h"

#pragma GCC diagnostic push
#pragma clang diagnostic ignored "-Wdisabled-macro-expansion"
#pragma clang diagnostic ignored "-Wused-but-marked-unused"
#pragma clang diagnostic ignored "-Wassign-enum"

typedef gboolean (*contact_tree_process)(ContactTree *, GtkTreeIter *, char *);

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

    /* Ignore the first line (header) */
    line = g_data_input_stream_read_line_utf8(data, &length, NULL, &err);
    g_free(line);

    do {
        line = g_data_input_stream_read_line_utf8(data, &length, NULL, &err);
        if (line != NULL) {
            contact_tree_line(tree, line);
        }
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

gboolean contact_tree_firstname(ContactTree *tree, GtkTreeIter *iter,
                                char *string) {
    GValue g_str = G_VALUE_INIT;
    g_value_init(&g_str, G_TYPE_STRING);
    g_value_set_static_string(&g_str, string);
    gtk_list_store_set_value(GTK_LIST_STORE(tree), iter, COLUMN_FIRSTNAME,
                             &g_str);

    return TRUE;
}

gboolean contact_tree_lastname(ContactTree *tree, GtkTreeIter *iter,
                               char *string) {
    GValue g_str = G_VALUE_INIT;
    g_value_init(&g_str, G_TYPE_STRING);
    g_value_set_static_string(&g_str, string);
    gtk_list_store_set_value(GTK_LIST_STORE(tree), iter, COLUMN_LASTNAME,
                             &g_str);

    return TRUE;
}

gboolean contact_tree_mail(ContactTree *tree, GtkTreeIter *iter, char *string) {
    GValue g_str = G_VALUE_INIT;
    g_value_init(&g_str, G_TYPE_STRING);
    g_value_set_static_string(&g_str, string);
    gtk_list_store_set_value(GTK_LIST_STORE(tree), iter, COLUMN_MAIL, &g_str);

    return TRUE;
}

gboolean contact_tree_phone(ContactTree *tree, GtkTreeIter *iter,
                            char *string) {
    GValue g_str = G_VALUE_INIT;
    g_value_init(&g_str, G_TYPE_STRING);
    g_value_set_static_string(&g_str, string);
    gtk_list_store_set_value(GTK_LIST_STORE(tree), iter, COLUMN_PHONE, &g_str);

    return TRUE;
}

gboolean contact_tree_birth(ContactTree *tree, GtkTreeIter *iter,
                            char *string) {
    GValue g_str = G_VALUE_INIT;
    g_value_init(&g_str, G_TYPE_STRING);
    g_value_set_static_string(&g_str, string);
    gtk_list_store_set_value(GTK_LIST_STORE(tree), iter, COLUMN_BIRTH, &g_str);

    return TRUE;
}

gboolean contact_tree_line(ContactTree *tree, char *line) {
    (void)tree;
    (void)line;
    static contact_tree_process callback_field[] = {
        contact_tree_lastname, contact_tree_firstname, contact_tree_mail,
        contact_tree_phone, contact_tree_birth};

    /* New iter */
    GtkTreeIter iter;
    gtk_list_store_append(GTK_LIST_STORE(tree), &iter);

    gchar **field = g_strsplit(line, ",", 5);

    for (int i = 0; i < 5; ++i) {
        callback_field[i](tree, &iter, field[i]);
    }

    g_strfreev(field);

    return TRUE;
}

#pragma GCC diagnostic pop
