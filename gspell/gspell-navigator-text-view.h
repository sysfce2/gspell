/*
 * This file is part of gspell, a spell-checking library.
 *
 * Copyright 2015, 2016 - Sébastien Wilmet <swilmet@gnome.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __GSPELL_NAVIGATOR_TEXT_VIEW_H__
#define __GSPELL_NAVIGATOR_TEXT_VIEW_H__

#if !defined (__GSPELL_H_INSIDE__) && !defined (GSPELL_COMPILATION)
#error "Only <gspell/gspell.h> can be included directly."
#endif

#include <gtk/gtk.h>
#include <gspell/gspell-navigator.h>
#include <gspell/gspell-checker.h>

G_BEGIN_DECLS

#define GSPELL_TYPE_NAVIGATOR_TEXT_VIEW (gspell_navigator_text_view_get_type ())
G_DECLARE_DERIVABLE_TYPE (GspellNavigatorTextView, gspell_navigator_text_view,
			  GSPELL, NAVIGATOR_TEXT_VIEW,
			  GObject)

struct _GspellNavigatorTextViewClass
{
	GObjectClass parent_class;

	/* Padding for future expansion */
	gpointer padding[8];
};

GspellNavigator *	gspell_navigator_text_view_new		(GtkTextView *view);

G_END_DECLS

#endif /* __GSPELL_NAVIGATOR_TEXT_VIEW_H__ */

/* ex:set ts=8 noet: */