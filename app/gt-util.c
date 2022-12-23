#include "gt-util.h"

void gt_util_transform_uris_to_quoted_fuse_paths(GStrv uris)
{
    guint i;

    if (!uris) {
        return;
    }

    for (i = 0; uris[i]; ++i) {
        g_autoptr (GFile) file = NULL;
        g_autofree char *path = NULL;

        file = g_file_new_for_uri (uris[i]);

        path = g_file_get_path (file);
        if (path) {
            char *quoted;

            quoted = g_shell_quote (path);
            g_free (uris[i]);

            uris[i] = quoted;
        }
    }
}


char* gt_util_concat_uris(GStrv uris, gsize *length)
{
    GString *string;
    gsize len;
    guint i;

    len = 0;
    for (i = 0; uris[i]; ++i) {
        len += strlen (uris[i]) + 1;
    }

    if (length) {
        *length = len;
    }

    string = g_string_sized_new (len + 1);
    for (i = 0; uris[i]; ++i) {
        g_string_append (string, uris[i]);
        g_string_append_c (string, ' ');
    }

    return g_string_free (string, FALSE);
}
