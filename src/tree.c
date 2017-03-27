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
    contact_tree_capitalize(tree, string);

    GValue g_str = G_VALUE_INIT;
    g_value_init(&g_str, G_TYPE_STRING);
    g_value_set_static_string(&g_str, string);

    gtk_list_store_set_value(GTK_LIST_STORE(tree), iter, COLUMN_FIRSTNAME,
                             &g_str);

    return TRUE;
}

gboolean contact_tree_lastname(ContactTree *tree, GtkTreeIter *iter,
                               char *string) {
    contact_tree_capitalize(tree, string);
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

static gboolean regex_remove(const GMatchInfo *info, GString *res,
                             gpointer data) {
    gchar *match = g_match_info_fetch(info, 0);

    g_free(match);
    (void)data;
    (void)res;
    return FALSE;
}

static gboolean regex_phone_indicator(const GMatchInfo *info, GString *res,
                                      gpointer data) {
    gchar *match = g_match_info_fetch(info, 0);

    g_string_append(res, "0");
    g_free(match);
    (void)data;
    return FALSE;
}

gboolean contact_tree_phone(ContactTree *tree, GtkTreeIter *iter,
                            char *string) {
    GValue g_str = G_VALUE_INIT;
    g_value_init(&g_str, G_TYPE_STRING);

    /* Remove delimiter : "." and whitespace */
    GRegex *regex_phone_delimiter = g_regex_new("(\\.|\\s|/)", 0, 0, NULL);
    char *phone_without_delimiter = g_regex_replace_eval(
        regex_phone_delimiter, string, -1, 0, 0, regex_remove, NULL, NULL);

    /* Remplace indicator by 0 */
    GRegex *regex_indicator = g_regex_new("^\\+33", 0, 0, NULL);
    char *phone =
        g_regex_replace_eval(regex_indicator, phone_without_delimiter, -1, 0, 0,
                             regex_phone_indicator, NULL, NULL);

    /* Check format phone */
    GMatchInfo *match_info;
    GRegex *regex_check = g_regex_new("^\\d{10}$", 0, 0, NULL);

    g_regex_match(regex_check, phone, 0, &match_info);
    if (g_match_info_matches(match_info)) {
        /* Insert phone in store  */
        g_value_set_static_string(&g_str, phone);
        gtk_list_store_set_value(GTK_LIST_STORE(tree), iter, COLUMN_PHONE,
                                 &g_str);
    } else {
        g_log("contact", G_LOG_LEVEL_WARNING, "Invalid number (%s)", phone);
        g_free(phone_without_delimiter);
        g_free(phone);
        return FALSE;
    }

    g_free(phone_without_delimiter);
    g_free(phone);

    return TRUE;
}

static GDateTime *str_to_date(char *string) {
    GDateTime *date = NULL;
    GRegex *regex;
    GMatchInfo *match_info;

    regex = g_regex_new("(\\d{4})-(\\d{2})-(\\d{2})", 0, 0, NULL);
    g_regex_match(regex, string, 0, &match_info);
    if (g_match_info_matches(match_info)) {
        gchar *year = g_match_info_fetch(match_info, 1);
        gchar *month = g_match_info_fetch(match_info, 2);
        gchar *day = g_match_info_fetch(match_info, 3);

        gint year_i = (gint)g_ascii_strtoll(year, NULL, 10);
        gint month_i = (gint)g_ascii_strtoll(month, NULL, 10);
        gint day_i = (gint)g_ascii_strtoll(day, NULL, 10);

        g_free(year);
        g_free(month);
        g_free(day);

        g_match_info_free(match_info);
        g_regex_unref(regex);

        date = g_date_time_new_local(year_i, month_i, day_i, 0, 0, 0);
    }

    return date;
}

gboolean contact_tree_birth(ContactTree *tree, GtkTreeIter *iter,
                            char *string) {
    GValue g_str = G_VALUE_INIT;
    g_value_init(&g_str, G_TYPE_STRING);
    GDateTime *date = str_to_date(string);
    ContactTreePrivate *priv;
    priv = contact_tree_get_instance_private(tree);

    if (date == NULL) {
        GtkDialogFlags flags = GTK_DIALOG_DESTROY_WITH_PARENT;
        GtkWidget *dialog =
            gtk_message_dialog_new(priv->win, flags, GTK_MESSAGE_ERROR,
                                   GTK_BUTTONS_CLOSE, "Invalid : %s", string);
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        return FALSE;
    }

    char *date_str = g_date_time_format(date, "%d/%m/%Y");
    g_value_set_static_string(&g_str, date_str);
    gtk_list_store_set_value(GTK_LIST_STORE(tree), iter, COLUMN_BIRTH, &g_str);

    return TRUE;
}

gboolean contact_tree_line(ContactTree *tree, char *line) {
    (void)tree;
    static contact_tree_process callback_field[] = {
        contact_tree_lastname, contact_tree_firstname, contact_tree_mail,
        contact_tree_phone, contact_tree_birth};

    /* New iter */
    GtkTreeIter iter;
    gtk_list_store_append(GTK_LIST_STORE(tree), &iter);

    gchar **field = g_strsplit(line, ",", 5);

    for (int i = 0; i < 5; ++i) {
        if (!callback_field[i](tree, &iter, field[i])) {
            g_log("contact", G_LOG_LEVEL_WARNING, "Error in a line : %s", line);
        }
    }

    g_strfreev(field);

    return TRUE;
}

static gboolean uppercase(const GMatchInfo *info, GString *res, gpointer data) {
    gchar *match = g_match_info_fetch(info, 0);

    g_string_append(res, g_utf8_strup(match, -1));
    g_free(match);
    (void)data;

    return FALSE;
}

gboolean contact_tree_capitalize(ContactTree *tree, char *string) {
    (void)tree;
    GRegex *regex = g_regex_new("^\\w", 0, 0, NULL);

    char *result =
        g_regex_replace_eval(regex, string, -1, 0, 0, uppercase, NULL, NULL);

    GRegex *regex2 = g_regex_new("-(\\w)", 0, 0, NULL);
    result =
        g_regex_replace_eval(regex2, result, -1, 0, 0, uppercase, NULL, NULL);

    glong length = g_utf8_strlen(result, -1);
    g_utf8_strncpy(string, result, (gsize)length);

    g_regex_unref(regex);

    return TRUE;
}

#pragma GCC diagnostic pop
