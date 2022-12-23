#include "gt-config.h"

#include <gio/gio.h>
#include <gdesktop-enums.h>

#include "gt-proxy-info.h"


struct _GtProxyInfo
{
    GObject parent_instance;

    GSettings *settings;
    GSettings *protocols[4];
    gulong changed_handler[4];

    GDesktopProxyMode mode;

    GHashTable *environ;
};


static char *proxy_scheme[] = {"http", "http", "http", "socks",};


static char *proxy_variable[] = {"http_proxy", "https_proxy", "ftp_proxy", "all_proxy",};


typedef enum
{
    HTTP = 0, HTTPS, FTP, SOCKS,
} Protocol;


G_DEFINE_TYPE (GtProxyInfo, gt_proxy_info, G_TYPE_OBJECT)


static void gt_proxy_info_dispose(GObject *object)
{
    GtProxyInfo *self = GT_PROXY_INFO (object);

    G_STATIC_ASSERT (G_N_ELEMENTS (self->protocols) == G_N_ELEMENTS (self->changed_handler));

    g_clear_object (&self->settings);

    for (int i = 0; i < G_N_ELEMENTS (self->protocols); i++) {
        g_clear_signal_handler (&self->changed_handler[i], self->protocols[i]);
        g_clear_object (&self->protocols[i]);
    }

    g_clear_pointer (&self->environ, g_hash_table_unref);

    G_OBJECT_CLASS (gt_proxy_info_parent_class)->dispose (object);
}


static void gt_proxy_info_class_init(GtProxyInfoClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->dispose = gt_proxy_info_dispose;
}


static void env_set_both(GtProxyInfo *self, const char *key, char *value)
{
    if (!value)
        return;

    g_hash_table_replace (self->environ, g_strdup (key), g_strdup (value));
    g_hash_table_replace (self->environ, g_ascii_strup (key, -1), value);
}


static void handle_protocol(GtProxyInfo *self, Protocol protocol)
{
    g_autoptr (GUri) uri = NULL;
    g_autofree char *host = NULL;
    g_autofree char *user = NULL;
    g_autofree char *password = NULL;
    int port = -1;
    GSettings *settings = self->protocols[protocol];

    host = g_settings_get_string (settings, "host");
    port = g_settings_get_int (settings, "port");

    if (!host || !host[0] || port == 0) {
        g_hash_table_remove (self->environ, proxy_variable[protocol]);
        return;
    }

    if (G_UNLIKELY (protocol == HTTP && g_settings_get_boolean (settings, "use-authentication"))) {
        user = g_settings_get_string (settings, "authentication-user");
        password = g_settings_get_string (settings, "authentication-password");
    }

    uri = g_uri_build_with_user (G_URI_FLAGS_NONE, proxy_scheme[protocol], user && user[0] ? user : NULL,
                                 password && password[0] ? password : NULL, NULL, host, port, "", NULL, NULL);

    env_set_both (self, proxy_variable[protocol], g_uri_to_string (uri));
}


static void handle_ignored(GtProxyInfo *self)
{
    g_autoptr (GString) value = NULL;
    g_auto (GStrv) hosts = NULL;

    g_settings_get (self->settings, "ignore-hosts", "^as", &hosts);
    if (hosts == NULL) {
        return;
    }

    value = g_string_sized_new (100);
    for (int i = 0; hosts[i] != NULL; ++i) {
        if (i > 0) {
            g_string_append_c (value, ',');
        }
        g_string_append (value, hosts[i]);
    }

    env_set_both (self, "no_proxy", g_string_free (g_steal_pointer (&value), FALSE));
}


static void manual_settings_changed(GSettings *settings, const char *key, GtProxyInfo *self)
{
    g_hash_table_remove_all (self->environ);

    for (int i = 0; i < G_N_ELEMENTS (self->protocols); i++) {
        handle_protocol (self, i);
    }

    handle_ignored (self);
}


static void proxy_settings_changed(GSettings *settings, const char *key, GtProxyInfo *self)
{
    GDesktopProxyMode mode = g_settings_get_enum (settings, "mode");

    if (mode == self->mode) {
        return;
    }

    switch (mode) {
        case G_DESKTOP_PROXY_MODE_MANUAL:
            for (int i = 0; i < G_N_ELEMENTS (self->changed_handler); i++) {
                self->changed_handler[i] = g_signal_connect (self->protocols[i], "changed",
                                                             G_CALLBACK (manual_settings_changed), self);
            }
            manual_settings_changed (NULL, NULL, self);
            break;
        case G_DESKTOP_PROXY_MODE_AUTO:
            g_info ("Can't handle auto proxy");
        case G_DESKTOP_PROXY_MODE_NONE:
            for (int i = 0; i < G_N_ELEMENTS (self->changed_handler); i++) {
                g_clear_signal_handler (&self->changed_handler[i], self->protocols[i]);
            }
            g_hash_table_remove_all (self->environ);
            break;
        default:
            g_return_if_reached ();
    }
}


static void gt_proxy_info_init(GtProxyInfo *self)
{
    self->settings = g_settings_new ("org.gnome.system.proxy");

    self->protocols[HTTP] = g_settings_get_child (self->settings, "http");
    self->protocols[HTTPS] = g_settings_get_child (self->settings, "https");
    self->protocols[FTP] = g_settings_get_child (self->settings, "ftp");
    self->protocols[SOCKS] = g_settings_get_child (self->settings, "socks");

    self->environ = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_free);

    g_signal_connect (self->settings, "changed", G_CALLBACK (proxy_settings_changed), self);
    proxy_settings_changed (self->settings, NULL, self);
}


GtProxyInfo *gt_proxy_info_get_default(void)
{
    static GtProxyInfo *instance;

    if (instance == NULL) {
        instance = g_object_new (GT_TYPE_PROXY_INFO, NULL);
        g_object_add_weak_pointer (G_OBJECT (instance), (gpointer *) &instance);
    }

    return instance;
}

void gt_proxy_info_apply_to_environ(GtProxyInfo *self, char ***env)
{
    GHashTableIter iter;
    char *key, *value;

    g_return_if_fail (GT_IS_PROXY_INFO (self));

    g_hash_table_iter_init (&iter, self->environ);
    while (g_hash_table_iter_next (&iter, (gpointer *) &key, (gpointer *) &value)) {
        *env = g_environ_setenv (*env, key, value, TRUE);
    }
}
