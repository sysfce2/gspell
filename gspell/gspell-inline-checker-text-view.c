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

#include "gspell-inline-checker-text-view.h"
#include "gspell-inline-checker-text-buffer.h"

/**
 * SECTION:inline-checker-text-view
 * @Short_description: Inline spell checker for GtkTextView
 * @Title: GspellInlineCheckerTextView
 *
 * The #GspellInlineCheckerTextView is an inline spell checker for the
 * #GtkTextView widget. Misspelled words are highlighted with a
 * %PANGO_UNDERLINE_ERROR, usually a red wavy underline. Right-clicking a
 * misspelled word pops up a context menu of suggested replacements. The context
 * menu also contains an “Ignore All” item to add the misspelled word to the
 * session dictionary. And an “Add” item to add the word to the personal
 * dictionary.
 *
 * The spell is checked only on the visible region of the #GtkTextView. Note
 * that if a same #GtkTextBuffer is used for several views, the misspelled words
 * are visible in all views, because the highlighting is achieved with a
 * #GtkTextTag added to the buffer.
 *
 * You need to call gspell_text_buffer_set_spell_checker() to associate a
 * #GspellChecker to the #GtkTextBuffer. #GtkTextView:buffer changes are
 * handled, as well as #GspellChecker changes.
 */

typedef struct _GspellInlineCheckerTextViewPrivate GspellInlineCheckerTextViewPrivate;

struct _GspellInlineCheckerTextViewPrivate
{
	GtkTextView *view;
	GspellInlineCheckerTextBuffer *buffer_checker;
};

enum
{
	PROP_0,
	PROP_VIEW,
	PROP_ENABLED,
};

#define INLINE_CHECKER_KEY "gspell-inline-checker-text-view-key"

G_DEFINE_TYPE_WITH_PRIVATE (GspellInlineCheckerTextView, gspell_inline_checker_text_view, G_TYPE_OBJECT)

static void
create_buffer_checker (GspellInlineCheckerTextView *inline_checker)
{
	GspellInlineCheckerTextViewPrivate *priv;
	GtkTextBuffer *buffer;

	priv = gspell_inline_checker_text_view_get_instance_private (inline_checker);

	if (priv->buffer_checker != NULL)
	{
		return;
	}

	buffer = gtk_text_view_get_buffer (priv->view);
	priv->buffer_checker = _gspell_inline_checker_text_buffer_new (buffer);
	_gspell_inline_checker_text_buffer_attach_view (priv->buffer_checker,
							priv->view);
}

static void
destroy_buffer_checker (GspellInlineCheckerTextView *inline_checker)
{
	GspellInlineCheckerTextViewPrivate *priv;

	priv = gspell_inline_checker_text_view_get_instance_private (inline_checker);

	if (priv->view == NULL || priv->buffer_checker == NULL)
	{
		return;
	}

	_gspell_inline_checker_text_buffer_detach_view (priv->buffer_checker,
							priv->view);
	g_clear_object (&priv->buffer_checker);
}

static void
notify_buffer_cb (GtkTextView                 *view,
		  GParamSpec                  *pspec,
		  GspellInlineCheckerTextView *inline_checker)
{
	GspellInlineCheckerTextViewPrivate *priv;

	priv = gspell_inline_checker_text_view_get_instance_private (inline_checker);

	if (priv->buffer_checker == NULL)
	{
		return;
	}

	destroy_buffer_checker (inline_checker);
	create_buffer_checker (inline_checker);
}

static void
set_view (GspellInlineCheckerTextView *inline_checker,
	  GtkTextView                 *view)
{
	GspellInlineCheckerTextViewPrivate *priv;

	g_return_if_fail (GTK_IS_TEXT_VIEW (view));

	priv = gspell_inline_checker_text_view_get_instance_private (inline_checker);

	g_assert (priv->view == NULL);
	g_assert (priv->buffer_checker == NULL);

	priv->view = view;

	g_signal_connect_object (priv->view,
				 "notify::buffer",
				 G_CALLBACK (notify_buffer_cb),
				 inline_checker,
				 0);
}

static void
gspell_inline_checker_text_view_get_property (GObject    *object,
					      guint       prop_id,
					      GValue     *value,
					      GParamSpec *pspec)
{
	GspellInlineCheckerTextView *inline_checker;
	GspellInlineCheckerTextViewPrivate *priv;

	inline_checker = GSPELL_INLINE_CHECKER_TEXT_VIEW (object);
	priv = gspell_inline_checker_text_view_get_instance_private (inline_checker);

	switch (prop_id)
	{
		case PROP_VIEW:
			g_value_set_object (value, priv->view);
			break;

		case PROP_ENABLED:
			g_value_set_boolean (value, gspell_inline_checker_text_view_get_enabled (inline_checker));
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
gspell_inline_checker_text_view_set_property (GObject      *object,
					      guint         prop_id,
					      const GValue *value,
					      GParamSpec   *pspec)
{
	GspellInlineCheckerTextView *inline_checker = GSPELL_INLINE_CHECKER_TEXT_VIEW (object);

	switch (prop_id)
	{
		case PROP_VIEW:
			set_view (inline_checker, g_value_get_object (value));
			break;

		case PROP_ENABLED:
			gspell_inline_checker_text_view_set_enabled (inline_checker,
								     g_value_get_boolean (value));
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
gspell_inline_checker_text_view_dispose (GObject *object)
{
	GspellInlineCheckerTextViewPrivate *priv;

	priv = gspell_inline_checker_text_view_get_instance_private (GSPELL_INLINE_CHECKER_TEXT_VIEW (object));

	if (priv->view != NULL && priv->buffer_checker != NULL)
	{
		_gspell_inline_checker_text_buffer_detach_view (priv->buffer_checker,
								priv->view);
	}

	priv->view = NULL;
	g_clear_object (&priv->buffer_checker);

	G_OBJECT_CLASS (gspell_inline_checker_text_view_parent_class)->dispose (object);
}

static void
gspell_inline_checker_text_view_class_init (GspellInlineCheckerTextViewClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->get_property = gspell_inline_checker_text_view_get_property;
	object_class->set_property = gspell_inline_checker_text_view_set_property;
	object_class->dispose = gspell_inline_checker_text_view_dispose;

	/**
	 * GspellInlineCheckerTextView:view:
	 *
	 * The #GtkTextView.
	 */
	g_object_class_install_property (object_class,
					 PROP_VIEW,
					 g_param_spec_object ("view",
							      "View",
							      "",
							      GTK_TYPE_TEXT_VIEW,
							      G_PARAM_READWRITE |
							      G_PARAM_CONSTRUCT_ONLY |
							      G_PARAM_STATIC_STRINGS));

	/**
	 * GspellInlineCheckerTextView:enabled:
	 *
	 * Whether the inline spell checker is enabled.
	 */
	g_object_class_install_property (object_class,
					 PROP_ENABLED,
					 g_param_spec_boolean ("enabled",
							       "Enabled",
							       "",
							       FALSE,
							       G_PARAM_READWRITE |
							       G_PARAM_STATIC_STRINGS));
}

static void
gspell_inline_checker_text_view_init (GspellInlineCheckerTextView *inline_checker)
{
}

/**
 * gspell_text_view_get_inline_checker:
 * @view: a #GtkTextView.
 *
 * Returns the #GspellInlineCheckerTextView of @view. The returned object is
 * guaranteed to be the same for the lifetime of @view.
 *
 * Returns: (transfer none): the #GspellInlineCheckerTextView of @view.
 */
GspellInlineCheckerTextView *
gspell_text_view_get_inline_checker (GtkTextView *view)
{
	GspellInlineCheckerTextView *inline_checker;

	g_return_val_if_fail (GTK_IS_TEXT_VIEW (view), NULL);

	inline_checker = g_object_get_data (G_OBJECT (view), INLINE_CHECKER_KEY);

	if (inline_checker == NULL)
	{
		inline_checker = g_object_new (GSPELL_TYPE_INLINE_CHECKER_TEXT_VIEW,
					       "view", view,
					       NULL);

		g_object_set_data_full (G_OBJECT (view),
					INLINE_CHECKER_KEY,
					inline_checker,
					g_object_unref);
	}

	return inline_checker;
}

/**
 * gspell_inline_checker_text_view_set_enabled:
 * @inline_checker: a #GspellInlineCheckerTextView.
 * @enabled: the new state.
 *
 * Enables or disables the inline spell checker.
 */
void
gspell_inline_checker_text_view_set_enabled (GspellInlineCheckerTextView *inline_checker,
					     gboolean                     enabled)
{
	g_return_if_fail (GSPELL_IS_INLINE_CHECKER_TEXT_VIEW (inline_checker));

	enabled = enabled != FALSE;

	if (enabled == gspell_inline_checker_text_view_get_enabled (inline_checker))
	{
		return;
	}

	if (enabled)
	{
		create_buffer_checker (inline_checker);
	}
	else
	{
		destroy_buffer_checker (inline_checker);
	}

	g_object_notify (G_OBJECT (inline_checker), "enabled");
}

/**
 * gspell_inline_checker_text_view_get_enabled:
 * @inline_checker: a #GspellInlineCheckerTextView.
 *
 * Returns: whether the inline spell checker is enabled.
 */
gboolean
gspell_inline_checker_text_view_get_enabled (GspellInlineCheckerTextView *inline_checker)
{
	GspellInlineCheckerTextViewPrivate *priv;

	g_return_val_if_fail (GSPELL_IS_INLINE_CHECKER_TEXT_VIEW (inline_checker), FALSE);

	priv = gspell_inline_checker_text_view_get_instance_private (inline_checker);

	return priv->buffer_checker != NULL;
}

/* ex:set ts=8 noet: */