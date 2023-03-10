/*
 * This file is generated by gdbus-codegen, do not modify it.
 *
 * The license of this code is the same as for the D-Bus interface description
 * it was derived from. Note that it links to GLib, so must comply with the
 * LGPL linking clauses.
 */

#ifndef __XDG_FM1_H__
#define __XDG_FM1_H__

#include <gio/gio.h>

G_BEGIN_DECLS


/* ------------------------------------------------------------------------ */
/* Declarations for org.freedesktop.FileManager1 */

#define XDG_TYPE_FILE_MANAGER1 (xdg_file_manager1_get_type ())
#define XDG_FILE_MANAGER1(o) (G_TYPE_CHECK_INSTANCE_CAST ((o), XDG_TYPE_FILE_MANAGER1, XdgFileManager1))
#define XDG_IS_FILE_MANAGER1(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), XDG_TYPE_FILE_MANAGER1))
#define XDG_FILE_MANAGER1_GET_IFACE(o) (G_TYPE_INSTANCE_GET_INTERFACE ((o), XDG_TYPE_FILE_MANAGER1, XdgFileManager1Iface))

struct _XdgFileManager1;
typedef struct _XdgFileManager1 XdgFileManager1;
typedef struct _XdgFileManager1Iface XdgFileManager1Iface;

struct _XdgFileManager1Iface
{
  GTypeInterface parent_iface;

  gboolean (*handle_show_folders) (
    XdgFileManager1 *object,
    GDBusMethodInvocation *invocation,
    const gchar *const *arg_URIs,
    const gchar *arg_StartupId);

  gboolean (*handle_show_item_properties) (
    XdgFileManager1 *object,
    GDBusMethodInvocation *invocation,
    const gchar *const *arg_URIs,
    const gchar *arg_StartupId);

  gboolean (*handle_show_items) (
    XdgFileManager1 *object,
    GDBusMethodInvocation *invocation,
    const gchar *const *arg_URIs,
    const gchar *arg_StartupId);

};

#if GLIB_CHECK_VERSION(2, 44, 0)
G_DEFINE_AUTOPTR_CLEANUP_FUNC (XdgFileManager1, g_object_unref)
#endif

GType xdg_file_manager1_get_type (void) G_GNUC_CONST;

GDBusInterfaceInfo *xdg_file_manager1_interface_info (void);
guint xdg_file_manager1_override_properties (GObjectClass *klass, guint property_id_begin);


/* D-Bus method call completion functions: */
void xdg_file_manager1_complete_show_folders (
    XdgFileManager1 *object,
    GDBusMethodInvocation *invocation);

void xdg_file_manager1_complete_show_items (
    XdgFileManager1 *object,
    GDBusMethodInvocation *invocation);

void xdg_file_manager1_complete_show_item_properties (
    XdgFileManager1 *object,
    GDBusMethodInvocation *invocation);



/* D-Bus method calls: */
void xdg_file_manager1_call_show_folders (
    XdgFileManager1 *proxy,
    const gchar *const *arg_URIs,
    const gchar *arg_StartupId,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data);

gboolean xdg_file_manager1_call_show_folders_finish (
    XdgFileManager1 *proxy,
    GAsyncResult *res,
    GError **error);

gboolean xdg_file_manager1_call_show_folders_sync (
    XdgFileManager1 *proxy,
    const gchar *const *arg_URIs,
    const gchar *arg_StartupId,
    GCancellable *cancellable,
    GError **error);

void xdg_file_manager1_call_show_items (
    XdgFileManager1 *proxy,
    const gchar *const *arg_URIs,
    const gchar *arg_StartupId,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data);

gboolean xdg_file_manager1_call_show_items_finish (
    XdgFileManager1 *proxy,
    GAsyncResult *res,
    GError **error);

gboolean xdg_file_manager1_call_show_items_sync (
    XdgFileManager1 *proxy,
    const gchar *const *arg_URIs,
    const gchar *arg_StartupId,
    GCancellable *cancellable,
    GError **error);

void xdg_file_manager1_call_show_item_properties (
    XdgFileManager1 *proxy,
    const gchar *const *arg_URIs,
    const gchar *arg_StartupId,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data);

gboolean xdg_file_manager1_call_show_item_properties_finish (
    XdgFileManager1 *proxy,
    GAsyncResult *res,
    GError **error);

gboolean xdg_file_manager1_call_show_item_properties_sync (
    XdgFileManager1 *proxy,
    const gchar *const *arg_URIs,
    const gchar *arg_StartupId,
    GCancellable *cancellable,
    GError **error);



/* ---- */

#define XDG_TYPE_FILE_MANAGER1_PROXY (xdg_file_manager1_proxy_get_type ())
#define XDG_FILE_MANAGER1_PROXY(o) (G_TYPE_CHECK_INSTANCE_CAST ((o), XDG_TYPE_FILE_MANAGER1_PROXY, XdgFileManager1Proxy))
#define XDG_FILE_MANAGER1_PROXY_CLASS(k) (G_TYPE_CHECK_CLASS_CAST ((k), XDG_TYPE_FILE_MANAGER1_PROXY, XdgFileManager1ProxyClass))
#define XDG_FILE_MANAGER1_PROXY_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), XDG_TYPE_FILE_MANAGER1_PROXY, XdgFileManager1ProxyClass))
#define XDG_IS_FILE_MANAGER1_PROXY(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), XDG_TYPE_FILE_MANAGER1_PROXY))
#define XDG_IS_FILE_MANAGER1_PROXY_CLASS(k) (G_TYPE_CHECK_CLASS_TYPE ((k), XDG_TYPE_FILE_MANAGER1_PROXY))

typedef struct _XdgFileManager1Proxy XdgFileManager1Proxy;
typedef struct _XdgFileManager1ProxyClass XdgFileManager1ProxyClass;
typedef struct _XdgFileManager1ProxyPrivate XdgFileManager1ProxyPrivate;

struct _XdgFileManager1Proxy
{
  /*< private >*/
  GDBusProxy parent_instance;
  XdgFileManager1ProxyPrivate *priv;
};

struct _XdgFileManager1ProxyClass
{
  GDBusProxyClass parent_class;
};

GType xdg_file_manager1_proxy_get_type (void) G_GNUC_CONST;

#if GLIB_CHECK_VERSION(2, 44, 0)
G_DEFINE_AUTOPTR_CLEANUP_FUNC (XdgFileManager1Proxy, g_object_unref)
#endif

void xdg_file_manager1_proxy_new (
    GDBusConnection     *connection,
    GDBusProxyFlags      flags,
    const gchar         *name,
    const gchar         *object_path,
    GCancellable        *cancellable,
    GAsyncReadyCallback  callback,
    gpointer             user_data);
XdgFileManager1 *xdg_file_manager1_proxy_new_finish (
    GAsyncResult        *res,
    GError             **error);
XdgFileManager1 *xdg_file_manager1_proxy_new_sync (
    GDBusConnection     *connection,
    GDBusProxyFlags      flags,
    const gchar         *name,
    const gchar         *object_path,
    GCancellable        *cancellable,
    GError             **error);

void xdg_file_manager1_proxy_new_for_bus (
    GBusType             bus_type,
    GDBusProxyFlags      flags,
    const gchar         *name,
    const gchar         *object_path,
    GCancellable        *cancellable,
    GAsyncReadyCallback  callback,
    gpointer             user_data);
XdgFileManager1 *xdg_file_manager1_proxy_new_for_bus_finish (
    GAsyncResult        *res,
    GError             **error);
XdgFileManager1 *xdg_file_manager1_proxy_new_for_bus_sync (
    GBusType             bus_type,
    GDBusProxyFlags      flags,
    const gchar         *name,
    const gchar         *object_path,
    GCancellable        *cancellable,
    GError             **error);


/* ---- */

#define XDG_TYPE_FILE_MANAGER1_SKELETON (xdg_file_manager1_skeleton_get_type ())
#define XDG_FILE_MANAGER1_SKELETON(o) (G_TYPE_CHECK_INSTANCE_CAST ((o), XDG_TYPE_FILE_MANAGER1_SKELETON, XdgFileManager1Skeleton))
#define XDG_FILE_MANAGER1_SKELETON_CLASS(k) (G_TYPE_CHECK_CLASS_CAST ((k), XDG_TYPE_FILE_MANAGER1_SKELETON, XdgFileManager1SkeletonClass))
#define XDG_FILE_MANAGER1_SKELETON_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), XDG_TYPE_FILE_MANAGER1_SKELETON, XdgFileManager1SkeletonClass))
#define XDG_IS_FILE_MANAGER1_SKELETON(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), XDG_TYPE_FILE_MANAGER1_SKELETON))
#define XDG_IS_FILE_MANAGER1_SKELETON_CLASS(k) (G_TYPE_CHECK_CLASS_TYPE ((k), XDG_TYPE_FILE_MANAGER1_SKELETON))

typedef struct _XdgFileManager1Skeleton XdgFileManager1Skeleton;
typedef struct _XdgFileManager1SkeletonClass XdgFileManager1SkeletonClass;
typedef struct _XdgFileManager1SkeletonPrivate XdgFileManager1SkeletonPrivate;

struct _XdgFileManager1Skeleton
{
  /*< private >*/
  GDBusInterfaceSkeleton parent_instance;
  XdgFileManager1SkeletonPrivate *priv;
};

struct _XdgFileManager1SkeletonClass
{
  GDBusInterfaceSkeletonClass parent_class;
};

GType xdg_file_manager1_skeleton_get_type (void) G_GNUC_CONST;

#if GLIB_CHECK_VERSION(2, 44, 0)
G_DEFINE_AUTOPTR_CLEANUP_FUNC (XdgFileManager1Skeleton, g_object_unref)
#endif

XdgFileManager1 *xdg_file_manager1_skeleton_new (void);


G_END_DECLS

#endif /* __XDG_FM1_H__ */
