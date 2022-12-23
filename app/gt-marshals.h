//
// Created by dingjing on 12/22/22.
//

#ifndef GRACEFUL_TERMINATOR_GT_MARSHALS_H
#define GRACEFUL_TERMINATOR_GT_MARSHALS_H

#include <glib-object.h>

G_BEGIN_DECLS

G_GNUC_INTERNAL
void gt_marshals_OBJECT__VOID (GClosure* closure, GValue* returnValue, guint nParamValues, const GValue* paramValues, gpointer invocationHint, gpointer mdata);

#define gt_marshals_VOID__ENUM	g_cclosure_marshal_VOID__ENUM

G_GNUC_INTERNAL
void gt_marshals_VOID__ENUM_STRING_BOOLEAN (GClosure* closure, GValue* returnValue, guint nParamValues, const GValue* paramValues, gpointer invocationHint, gpointer marshalData);

G_GNUC_INTERNAL
void gt_marshals_VOID__UINT_UINT (GClosure* closure, GValue* returnValue, guint nParamValues, const GValue* paramValues, gpointer invocationHint, gpointer marshalData);

#define gt_marshals_VOID__VOID      g_cclosure_marshal_VOID__VOID

G_END_DECLS

#endif //GRACEFUL_TERMINATOR_GT_MARSHALS_H
