/*
 * This file is part of gspell, a spell-checking library.
 *
 * Copyright 2015, 2016 - Sébastien Wilmet
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

#include "gspell-text-view.h"
#include "gspell-inline-checker-text.h"

#define INLINE_CHECKER_KEY "gspell-text-view-inline-checker-key"

/**
 * gspell_text_view_get_inline_checker:
 * @view: a #GtkTextView.
 *
 * Returns the #GspellInlineCheckerText of @view. The returned object is
 * guaranteed to be the same for the lifetime of @view.
 *
 * Returns: (transfer none): the #GspellInlineCheckerText of @view.
 */
GspellInlineCheckerText *
gspell_text_view_get_inline_checker (GtkTextView *view)
{
	GspellInlineCheckerText *inline_checker;

	g_return_val_if_fail (GTK_IS_TEXT_VIEW (view), NULL);

	inline_checker = g_object_get_data (G_OBJECT (view), INLINE_CHECKER_KEY);

	if (inline_checker == NULL)
	{
		inline_checker = _gspell_inline_checker_text_new (view);

		g_object_set_data_full (G_OBJECT (view),
					INLINE_CHECKER_KEY,
					inline_checker,
					g_object_unref);
	}

	return inline_checker;
}

/* ex:set ts=8 noet: */
