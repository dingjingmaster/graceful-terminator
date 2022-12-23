#include "gt-marshals.h"

#include <glib-object.h>

#ifdef G_ENABLE_DEBUG
#define g_marshal_value_peek_boolean(v)  g_value_get_boolean (v)
#define g_marshal_value_peek_char(v)     g_value_get_schar (v)
#define g_marshal_value_peek_uchar(v)    g_value_get_uchar (v)
#define g_marshal_value_peek_int(v)      g_value_get_int (v)
#define g_marshal_value_peek_uint(v)     g_value_get_uint (v)
#define g_marshal_value_peek_long(v)     g_value_get_long (v)
#define g_marshal_value_peek_ulong(v)    g_value_get_ulong (v)
#define g_marshal_value_peek_int64(v)    g_value_get_int64 (v)
#define g_marshal_value_peek_uint64(v)   g_value_get_uint64 (v)
#define g_marshal_value_peek_enum(v)     g_value_get_enum (v)
#define g_marshal_value_peek_flags(v)    g_value_get_flags (v)
#define g_marshal_value_peek_float(v)    g_value_get_float (v)
#define g_marshal_value_peek_double(v)   g_value_get_double (v)
#define g_marshal_value_peek_string(v)   (char*) g_value_get_string (v)
#define g_marshal_value_peek_param(v)    g_value_get_param (v)
#define g_marshal_value_peek_boxed(v)    g_value_get_boxed (v)
#define g_marshal_value_peek_pointer(v)  g_value_get_pointer (v)
#define g_marshal_value_peek_object(v)   g_value_get_object (v)
#define g_marshal_value_peek_variant(v)  g_value_get_variant (v)
#else /* !G_ENABLE_DEBUG */
#define g_marshal_value_peek_boolean(v)  (v)->data[0].v_int
#define g_marshal_value_peek_char(v)     (v)->data[0].v_int
#define g_marshal_value_peek_uchar(v)    (v)->data[0].v_uint
#define g_marshal_value_peek_int(v)      (v)->data[0].v_int
#define g_marshal_value_peek_uint(v)     (v)->data[0].v_uint
#define g_marshal_value_peek_long(v)     (v)->data[0].v_long
#define g_marshal_value_peek_ulong(v)    (v)->data[0].v_ulong
#define g_marshal_value_peek_int64(v)    (v)->data[0].v_int64
#define g_marshal_value_peek_uint64(v)   (v)->data[0].v_uint64
#define g_marshal_value_peek_enum(v)     (v)->data[0].v_long
#define g_marshal_value_peek_flags(v)    (v)->data[0].v_ulong
#define g_marshal_value_peek_float(v)    (v)->data[0].v_float
#define g_marshal_value_peek_double(v)   (v)->data[0].v_double
#define g_marshal_value_peek_string(v)   (v)->data[0].v_pointer
#define g_marshal_value_peek_param(v)    (v)->data[0].v_pointer
#define g_marshal_value_peek_boxed(v)    (v)->data[0].v_pointer
#define g_marshal_value_peek_pointer(v)  (v)->data[0].v_pointer
#define g_marshal_value_peek_object(v)   (v)->data[0].v_pointer
#define g_marshal_value_peek_variant(v)  (v)->data[0].v_pointer
#endif /* !G_ENABLE_DEBUG */

void gt_marshals_OBJECT__VOID (GClosure* closure, GValue* returnValue, guint nParamValues, const GValue* paramValues, gpointer invocationHint, gpointer mdata)
{
    typedef GObject* (*GMarshalFunc_OBJECT__VOID) (gpointer data1, gpointer data2);
    GCClosure *cc = (GCClosure *) closure;
    gpointer data1, data2;
    GMarshalFunc_OBJECT__VOID callback;

    g_return_if_fail (returnValue != NULL);
    g_return_if_fail (nParamValues == 1);

    if (G_CCLOSURE_SWAP_DATA (closure)) {
        data1 = closure->data;
        data2 = g_value_peek_pointer (paramValues + 0);
    }
    else {
        data1 = g_value_peek_pointer (paramValues + 0);
        data2 = closure->data;
    }

    callback = (GMarshalFunc_OBJECT__VOID) (mdata ? mdata : cc->callback);

    GObject* vReturn = callback (data1, data2);

    g_value_take_object (returnValue, vReturn);
}

void gt_marshals_VOID__ENUM_STRING_BOOLEAN (GClosure* closure, GValue* returnValue, guint nParamValues, const GValue* paramValues, gpointer invocationHint, gpointer marshalData)
{
    typedef void (*GMarshalFunc_VOID__ENUM_STRING_BOOLEAN) (gpointer data1, gint arg1, gpointer arg2, gboolean arg3, gpointer data2);
    GCClosure *cc = (GCClosure *) closure;
    gpointer data1, data2;
    GMarshalFunc_VOID__ENUM_STRING_BOOLEAN callback;

    g_return_if_fail (nParamValues == 4);

    if (G_CCLOSURE_SWAP_DATA (closure)) {
        data1 = closure->data;
        data2 = g_value_peek_pointer (paramValues + 0);
    }
    else {
        data1 = g_value_peek_pointer (paramValues + 0);
        data2 = closure->data;
    }

    callback = (GMarshalFunc_VOID__ENUM_STRING_BOOLEAN) (marshalData ? marshalData : cc->callback);
    callback (data1, g_marshal_value_peek_enum (paramValues + 1), g_marshal_value_peek_string (paramValues + 2), g_marshal_value_peek_boolean (paramValues + 3), data2);
}

void gt_marshals_VOID__UINT_UINT (GClosure* closure, GValue* returnValue, guint nParamValues, const GValue* paramValues, gpointer invocationHint, gpointer marshalData)
{
    typedef void (*GMarshalFunc_VOID__UINT_UINT) (gpointer data1, guint arg1, guint arg2, gpointer data2);
    GCClosure *cc = (GCClosure *) closure;
    gpointer data1, data2;
    GMarshalFunc_VOID__UINT_UINT callback;

    g_return_if_fail (nParamValues == 3);

    if (G_CCLOSURE_SWAP_DATA (closure)) {
        data1 = closure->data;
        data2 = g_value_peek_pointer (paramValues + 0);
    }
    else {
        data1 = g_value_peek_pointer (paramValues + 0);
        data2 = closure->data;
    }

    callback = (GMarshalFunc_VOID__UINT_UINT) (marshalData ? marshalData : cc->callback);
    callback (data1, g_marshal_value_peek_uint (paramValues + 1), g_marshal_value_peek_uint (paramValues + 2), data2);
}
