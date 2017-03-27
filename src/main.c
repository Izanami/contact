#include "contactapp.h"
#include "gtk.h"

#pragma GCC diagnostic push
#pragma clang diagnostic ignored "-Wassign-enum"

static void log_handler(const gchar *log_domain, GLogLevelFlags log_level,
                        const gchar *message, gpointer user_data) {
    (void)log_domain;
    (void)log_level;
    (void)user_data;
    GFile *file = g_file_new_for_path("./contact.log");
    GFileOutputStream *stream =
        g_file_append_to(file, G_FILE_CREATE_PRIVATE, NULL, NULL);

    gchar *line = g_strconcat(message, "\n", NULL);

    g_output_stream_write(G_OUTPUT_STREAM(stream), line,
                          (gsize)g_utf8_strlen(line, -1), NULL, NULL);

    g_object_unref(file);
    g_object_unref(stream);
    g_free(line);
}

int main(int argc, char **argv) {
    g_log_set_handler("contact", G_LOG_LEVEL_WARNING, log_handler, NULL);
    return g_application_run(G_APPLICATION(contact_app_new()), argc, argv);
}

#pragma GCC diagnostic pop
