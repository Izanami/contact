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

    // Initialize super
    GtkListStore *list = GTK_LIST_STORE(tree);
    GType column[] = {G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,
                      G_TYPE_STRING, G_TYPE_STRING};

    gtk_list_store_set_column_types(list, N_COLUMNS, column);
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

    // Ignore the first line (header)
    line = g_data_input_stream_read_line_utf8(data, &length, NULL, &err);
    g_free(line);

    do {
        line = g_data_input_stream_read_line_utf8(data, &length, NULL, &err);
        if (line != NULL) {
            contact_tree_line(tree, line);
        }
        g_free(line);
    } while (line != NULL);

    g_input_stream_close(G_INPUT_STREAM(stream), NULL, NULL);

    contact_tree_write(tree, file);

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
    (void)data;
    (void)res;

    gchar *match = g_match_info_fetch(info, 0);
    g_free(match);

    return FALSE;
}

static gboolean regex_phone_indicator(const GMatchInfo *info, GString *res,
                                      gpointer data) {
    (void)data;

    gchar *match = g_match_info_fetch(info, 0);

    g_string_append(res, "0");
    g_free(match);

    return FALSE;
}

gboolean contact_tree_phone(ContactTree *tree, GtkTreeIter *iter,
                            char *string) {
    GValue g_str = G_VALUE_INIT;
    g_value_init(&g_str, G_TYPE_STRING);

    // Remove delimiter : "." and whitespace
    GRegex *regex_delimiter = g_regex_new("(\\.|\\s|/)", 0, 0, NULL);
    char *phone_without_delimiter = g_regex_replace_eval(
        regex_delimiter, string, -1, 0, 0, regex_remove, NULL, NULL);

    // Remplace indicator by 0
    GRegex *regex_indicator = g_regex_new("^\\+33", 0, 0, NULL);
    char *phone =
        g_regex_replace_eval(regex_indicator, phone_without_delimiter, -1, 0, 0,
                             regex_phone_indicator, NULL, NULL);

    // Check format phone
    GMatchInfo *match_info;
    GRegex *regex_check = g_regex_new("^\\d{10}$", 0, 0, NULL);

    g_regex_match(regex_check, phone, 0, &match_info);
    if (g_match_info_matches(match_info)) {
        // Insert phone in store
        g_value_set_static_string(&g_str, phone);
        gtk_list_store_set_value(GTK_LIST_STORE(tree), iter, COLUMN_PHONE,
                                 &g_str);
    } else {
        g_log("contact", G_LOG_LEVEL_WARNING, "Invalid number (%s)", phone);
    }

    // Free memory
    g_match_info_free(match_info);
    g_regex_unref(regex_delimiter);
    g_regex_unref(regex_indicator);
    g_regex_unref(regex_check);
    g_free(phone_without_delimiter);
    g_free(phone);

    return g_match_info_matches(match_info);
}

static GDateTime *str_to_date(char *string) {
    GDateTime *date = NULL;
    GRegex *regex;
    GMatchInfo *match_info;

    // Parse date
    regex = g_regex_new("(\\d{4})-(\\d{2})-(\\d{2})", 0, 0, NULL);
    g_regex_match(regex, string, 0, &match_info);
    if (g_match_info_matches(match_info)) {
        // Get matchs
        gchar *year = g_match_info_fetch(match_info, 1);
        gchar *month = g_match_info_fetch(match_info, 2);
        gchar *day = g_match_info_fetch(match_info, 3);

        // Convert to integer
        gint year_i = (gint)g_ascii_strtoll(year, NULL, 10);
        gint month_i = (gint)g_ascii_strtoll(month, NULL, 10);
        gint day_i = (gint)g_ascii_strtoll(day, NULL, 10);

        // Freee memory
        g_free(year);
        g_free(month);
        g_free(day);

        g_match_info_free(match_info);
        g_regex_unref(regex);

        // Create nex date
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

    // If error dialog error
    if (date == NULL) {
        GtkDialogFlags flags = GTK_DIALOG_DESTROY_WITH_PARENT;
        GtkWidget *dialog =
            gtk_message_dialog_new(priv->win, flags, GTK_MESSAGE_ERROR,
                                   GTK_BUTTONS_CLOSE, "Invalid : %s", string);
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        return FALSE;
    }

    // Insert date in store
    char *date_str = g_date_time_format(date, "%d/%m/%Y");
    g_value_set_static_string(&g_str, date_str);
    gtk_list_store_set_value(GTK_LIST_STORE(tree), iter, COLUMN_BIRTH, &g_str);

    // Free memory
    g_date_time_unref(date);
    g_free(date_str);

    return TRUE;
}

gboolean contact_tree_null(ContactTree *tree, GtkTreeIter *iter, char *string) {
    (void)tree;
    (void)iter;
    (void)string;

    return TRUE;
}

gboolean contact_tree_line(ContactTree *tree, char *line) {
    static contact_tree_process callback_field[] = {
        contact_tree_lastname, contact_tree_firstname, contact_tree_null,
        contact_tree_phone, contact_tree_birth};

    // New iter
    GtkTreeIter iter;
    gtk_list_store_append(GTK_LIST_STORE(tree), &iter);

    // Split
    gchar **field = g_strsplit(line, ",", 5);

    // For each field call process
    for (unsigned long i = 0;
         i < sizeof(callback_field) / sizeof(contact_tree_process); ++i) {
        if (!callback_field[i](tree, &iter, field[i])) {
            g_log("contact", G_LOG_LEVEL_WARNING, "Error in a line : %s", line);
        }
    }

    // Generate mail
    contact_tree_mail_generate(tree, &iter);

    // Free memory
    g_strfreev(field);

    return TRUE;
}

static gboolean uppercase(const GMatchInfo *info, GString *res, gpointer data) {
    (void)data;

    gchar *match = g_match_info_fetch(info, 0);  // Get match
    char *upper = g_utf8_strup(match, -1);
    g_string_append(res, upper);

    // Free memory
    g_free(match);
    g_free(upper);

    return FALSE;
}

gboolean contact_tree_capitalize(ContactTree *tree, char *string) {
    (void)tree;
    GRegex *regex_begin = g_regex_new("^\\w", 0, 0, NULL);
    GRegex *regex_inter = g_regex_new("-(\\w)", 0, 0, NULL);

    char *result = g_regex_replace_eval(regex_begin, string, -1, 0, 0,
                                        uppercase, NULL, NULL);

    char *result2 = g_regex_replace_eval(regex_inter, result, -1, 0, 0,
                                         uppercase, NULL, NULL);

    // Copy the uppercase string
    glong length = g_utf8_strlen(result2, -1);
    g_utf8_strncpy(string, result2, (gsize)length);

    // Free memory
    g_free(result);
    g_free(result2);
    g_regex_unref(regex_begin);
    g_regex_unref(regex_inter);

    return TRUE;
}

gboolean contact_tree_mail_generate(ContactTree *tree, GtkTreeIter *iter) {
    gchar *firstname;
    gchar *lastname;

    // Get datas in store
    gtk_tree_model_get(GTK_TREE_MODEL(tree), iter, COLUMN_FIRSTNAME, &firstname,
                       COLUMN_LASTNAME, &lastname, -1);

    // To lowercase
    gchar *firstname_l = g_utf8_strdown(firstname, -1);
    gchar *lastname_l = g_utf8_strdown(lastname, -1);

    // Only ascii letters
    gchar *firstname_a = g_str_to_ascii(firstname_l, NULL);
    gchar *lastname_a = g_str_to_ascii(lastname_l, NULL);

    // Insert generated mail
    GValue g_str = G_VALUE_INIT;
    g_value_init(&g_str, G_TYPE_STRING);
    gchar *string =
        g_strconcat(firstname_a, ".", lastname_a, "@viacesi.fr", NULL);

    g_value_set_static_string(&g_str, string);
    gtk_list_store_set_value(GTK_LIST_STORE(tree), iter, COLUMN_MAIL, &g_str);

    // Free memory
    g_free(string);
    g_free(firstname);
    g_free(firstname_l);
    g_free(firstname_a);
    g_free(lastname);
    g_free(lastname_l);
    g_free(lastname_a);

    return TRUE;
}

gboolean contact_tree_write(ContactTree *tree, GFile *file) {
    ContactTreePrivate *priv;
    priv = contact_tree_get_instance_private(tree);

    GError *err = NULL;

    /*GFile *f = g_file_new_for_path("./data.csv");*/
    GFileOutputStream *output =
        g_file_replace(file, NULL, FALSE, G_FILE_CREATE_NONE, NULL, &err);

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

    gint a = 1;
    gchar *buf = g_strdup_printf(" %d", a);
    g_output_stream_write(G_OUTPUT_STREAM(output), buf, 2, NULL, NULL);
    g_output_stream_close(G_OUTPUT_STREAM(output), NULL, NULL);
    g_free(buf);
    g_object_unref(output);
    /*g_object_unref(f);*/

    (void)tree;
    (void)file;
    return TRUE;
}

#pragma GCC diagnostic pop
