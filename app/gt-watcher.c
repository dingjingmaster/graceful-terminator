#include "gt-watcher.h"

#include "gt-config.h"

struct ProcessWatch
{
    GtTab *page;
    GtProcess *process;
};


struct _GtWatcher
{
    GObject parent_instance;

    GTree *watching;
    GTree *children;

    guint timeout;
    int active;
};


G_DEFINE_TYPE (GtWatcher, gt_watcher, G_TYPE_OBJECT)


static void gt_watcher_dispose(GObject *object)
{
    GtWatcher *self = GT_WATCHER (object);

    g_clear_pointer (&self->watching, g_tree_unref);
    g_clear_pointer (&self->children, g_tree_unref);

    G_OBJECT_CLASS (gt_watcher_parent_class)->dispose (object);
}


static void gt_watcher_class_init(GtWatcherClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->dispose = gt_watcher_dispose;
}


static void clear_watch(struct ProcessWatch *watch)
{
    g_return_if_fail (watch != NULL);

    g_clear_pointer (&watch->process, gt_process_unref);
    g_clear_weak_pointer (&watch->page);

    g_clear_pointer (&watch, g_free);
}


static gboolean handle_watch_iter(gpointer pid, gpointer val, gpointer user_data)
{
    GtProcess *process = val;
    GtWatcher *self = user_data;
    GPid parent = gt_process_get_parent (process);
    struct ProcessWatch *watch = NULL;

    watch = g_tree_lookup (self->watching, GINT_TO_POINTER (parent));

    // There are far more processes on the system than there are children
    // of watches, thus lookup are unlikly
    if (G_UNLIKELY (watch != NULL)) {

        /* If the page died we stop caring about its processes */
        if (G_UNLIKELY (watch->page == NULL)) {
            g_tree_remove (self->watching, GINT_TO_POINTER (parent));
            g_tree_remove (self->children, pid);

            return FALSE;
        }

        if (!g_tree_lookup (self->children, pid)) {
            struct ProcessWatch *child_watch = g_new0 (struct ProcessWatch, 1);

            child_watch->process = g_rc_box_acquire (process);
            g_set_weak_pointer (&child_watch->page, watch->page);

            g_debug ("Hello %i!", GPOINTER_TO_INT (pid));

            g_tree_insert (self->children, pid, child_watch);
        }

        gt_tab_push_child (watch->page, process);
    }

    return FALSE;
}


struct RemoveDead
{
    GTree *plist;
    GPtrArray *dead;
};


static gboolean remove_dead(gpointer pid, gpointer val, gpointer user_data)
{
    struct RemoveDead *data = user_data;
    struct ProcessWatch *watch = val;

    if (!g_tree_lookup (data->plist, pid)) {
        g_debug ("%i marked as dead", GPOINTER_TO_INT (pid));

        gt_tab_pop_child (watch->page, watch->process);

        g_ptr_array_add (data->dead, pid);
    }

    return FALSE;
}


static gboolean watch(gpointer data)
{
    GtWatcher *self = GT_WATCHER (data);
    g_autoptr (GTree) plist = NULL;
    struct RemoveDead dead;

    plist = gt_process_get_list ();

    g_tree_foreach (plist, handle_watch_iter, self);

    dead.plist = plist;
    dead.dead = g_ptr_array_new_full (1, NULL);

    g_tree_foreach (self->children, remove_dead, &dead);

    // We can't modify self->children whilst walking it
    for (int i = 0; i < dead.dead->len; i++) {
        g_tree_remove (self->children, g_ptr_array_index (dead.dead, i));
    }

    g_ptr_array_unref (dead.dead);

    return G_SOURCE_CONTINUE;
}


static inline void set_watcher(GtWatcher *self, gboolean focused)
{
    g_debug ("updated watcher focused? %s", focused ? "yes" : "no");

    if (self->timeout != 0) {
        g_source_remove (self->timeout);
    }

    // Slow down polling when nothing is focused
    self->timeout = g_timeout_add (focused ? 500 : 2000, watch, self);
    g_source_set_name_by_id (self->timeout, "[gt] child watcher");
}


static void gt_watcher_init(GtWatcher *self)
{
    self->watching = g_tree_new_full (gt_pid_cmp, NULL, NULL, (GDestroyNotify) clear_watch);
    self->children = g_tree_new_full (gt_pid_cmp, NULL, NULL, (GDestroyNotify) clear_watch);

    self->active = 0;
    self->timeout = 0;

    set_watcher (self, TRUE);
}

GtWatcher *gt_watcher_get_default(void)
{
    static GtWatcher *instance;

    if (instance == NULL) {
        instance = g_object_new (GT_TYPE_WATCHER, NULL);
        g_object_add_weak_pointer (G_OBJECT (instance), (gpointer *) &instance);
    }

    return instance;
}

void gt_watcher_add(GtWatcher *self, GPid pid, GtTab *page)
{
    struct ProcessWatch *watch;

    g_return_if_fail (GT_IS_WATCHER (self));
    g_return_if_fail (GT_IS_TAB (page));

    watch = g_new0 (struct ProcessWatch, 1);
    watch->process = gt_process_new (pid);
    g_set_weak_pointer (&watch->page, page);

    g_debug ("Started watching %i", pid);

    g_tree_insert (self->watching, GINT_TO_POINTER (pid), watch);
}

void gt_watcher_remove(GtWatcher *self, GPid pid)
{
    g_return_if_fail (GT_IS_WATCHER (self));

    if (G_LIKELY (g_tree_lookup (self->watching, GINT_TO_POINTER (pid)))) {
        g_tree_remove (self->watching, GINT_TO_POINTER (pid));
        g_debug ("Stopped watching %i", pid);
    } else {
        g_warning ("Unknown process %i", pid);
    }
}


void gt_watcher_push_active(GtWatcher *self)
{
    g_return_if_fail (GT_IS_WATCHER (self));

    self->active++;

    g_debug ("push_active");

    if (G_LIKELY (self->active > 0)) {
        set_watcher (self, TRUE);
    } else {
        set_watcher (self, FALSE);
    }
}


void gt_watcher_pop_active(GtWatcher *self)
{
    g_return_if_fail (GT_IS_WATCHER (self));

    self->active--;

    g_debug ("pop_active");

    if (G_LIKELY (self->active < 1)) {
        set_watcher (self, FALSE);
    } else {
        set_watcher (self, TRUE);
    }
}

