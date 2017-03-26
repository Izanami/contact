#include "gtk.h"

#include "tree.h"

#pragma GCC diagnostic push
#pragma clang diagnostic ignored "-Wdisabled-macro-expansion"

/// \brief Window class
struct _ContactTree {
    GtkListStore parent;  ///< Parent
};

G_DEFINE_TYPE(ContactTree, contact_tree, GTK_TYPE_LIST_STORE)

static void contact_tree_init(ContactTree *tree) {
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
    (void)tree;
    GError *err = NULL;
    GFileInputStream *stream = g_file_read(file, NULL, &err);
    GDataInputStream *data = g_data_input_stream_new(G_INPUT_STREAM(stream));

    gsize length;

    char *line = g_data_input_stream_read_line_utf8(data, &length, NULL, &err);
    g_free(line);

    return TRUE;
}

#pragma GCC diagnostic pop
