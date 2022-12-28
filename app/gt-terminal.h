//
// Created by dingjing on 12/22/22.
//

#ifndef GRACEFUL_TERMINATOR_GT_TERMINAL_H
#define GRACEFUL_TERMINATOR_GT_TERMINAL_H

#include <glib-object.h>
#include <vte/vte.h>

G_BEGIN_DECLS

#define GT_TYPE_TERMINAL gt_terminal_get_type()

G_DECLARE_FINAL_TYPE (GtTerminal, gt_terminal, GT, TERMINAL, VteTerminal)

void gt_terminal_accept_paste (GtTerminal* self, const char* text);

G_END_DECLS

#endif //GRACEFUL_TERMINATOR_GT_TERMINAL_H
