/*
 * This file is part of gspell, a spell-checking library.
 *
 * Copyright 2006 - Paolo Maggi
 * Copyright 2008 - Novell, Inc.
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "gspell-language.h"
#include <string.h>
#include <glib/gi18n-lib.h>
#include <enchant.h>
#include "gspell-init.h"
#include "gspell-utils.h"

#ifdef OS_OSX
#include "gspell-osx.h"
#endif

/**
 * SECTION:language
 * @Short_description: Language
 * @Title: GspellLanguage
 * @See_also: #GspellChecker
 *
 * #GspellLanguage represents a language that can be used for the spell
 * checking, i.e. a language for which a dictionary is installed.
 */

struct _GspellLanguage
{
	gchar *code;
	gchar *name;
	gchar *collate_key;
};

G_DEFINE_BOXED_TYPE (GspellLanguage,
		     gspell_language,
		     gspell_language_copy,
		     gspell_language_free)

static gboolean
_gspell_language_traverse_cb (const gchar  *code,
			      const gchar  *name,
			      GList       **available_languages)
{
	GspellLanguage *language;

	language = g_slice_new (GspellLanguage);
	language->code = g_strdup (code);
	language->name = g_strdup (name);
	language->collate_key = g_utf8_collate_key (name, -1);

	*available_languages = g_list_insert_sorted (*available_languages,
						     language,
						     (GCompareFunc) gspell_language_compare);

	return FALSE;
}

static gint
tree_compare_func (gconstpointer a,
		   gconstpointer b,
		   gpointer      user_data)
{
	return g_strcmp0 (a, b);
}

/**
 * gspell_language_get_available:
 *
 * Returns: (transfer none) (element-type GspellLanguage): the list of available
 * languages, sorted with gspell_language_compare().
 */
const GList *
gspell_language_get_available (void)
{
	static gboolean initialized = FALSE;
	static GList *available_languages = NULL;
	DictsData data;
	EnchantBroker *broker;

	if (initialized)
	{
		return available_languages;
	}

	initialized = TRUE;

	_gspell_populate_language_ht (&data);
	data.tree = g_tree_new_full (tree_compare_func,
				     NULL,
				     (GDestroyNotify) g_free,
				     (GDestroyNotify) g_free);

	broker = enchant_broker_init ();

	enchant_broker_list_dicts (broker,
				   (EnchantDictDescribeFn) _gspell_language_dict_describe_cb,
				   &data);
	enchant_broker_free (broker);

	g_tree_foreach (data.tree,
			(GTraverseFunc) _gspell_language_traverse_cb,
			&available_languages);

	/* Note: we do use iso_639_table on Windows! */
	g_hash_table_unref (data.iso_639_table);

#ifndef G_OS_WIN32
	g_hash_table_unref (data.iso_3166_table);
#endif

	g_tree_unref (data.tree);

	return available_languages;
}

/**
 * gspell_language_get_default:
 *
 * Finds the best available language based on the current locale.
 *
 * Returns: (nullable): the default #GspellLanguage, or %NULL if no dictionaries
 * are available.
 */
const GspellLanguage *
gspell_language_get_default (void)
{
	const GspellLanguage *lang;
	const gchar * const *lang_names;
	const GList *langs;
	gint i;

	/* Try with the current locale */
	lang_names = g_get_language_names ();

	for (i = 0; lang_names[i] != NULL; i++)
	{
		lang = gspell_language_lookup (lang_names[i]);

		if (lang != NULL)
		{
			return lang;
		}
	}

	/* Another try specific to Mac OS X */
#ifdef OS_OSX
	{
		gchar *code = _gspell_osx_get_preferred_spell_language ();

		if (code != NULL)
		{
			lang = gspell_language_lookup (code);
			g_free (code);

			if (lang != NULL)
			{
				return lang;
			}
		}
	}
#endif

	/* Try English */
	lang = gspell_language_lookup ("en_US");
	if (lang != NULL)
	{
		return lang;
	}

	/* Take the first available language */
	langs = gspell_language_get_available ();
	if (langs != NULL)
	{
		return langs->data;
	}

	return NULL;
}

static GspellLanguage *
lookup_language_internal (const gchar *language_code,
                          gboolean     use_closest_match)
{
	const GspellLanguage *closest_match = NULL;
	const GList *available_languages;
	const GList *l;

	g_return_val_if_fail (language_code != NULL, NULL);

	available_languages = gspell_language_get_available ();

	for (l = available_languages; l != NULL; l = l->next)
	{
		const GspellLanguage *language = l->data;
		const gchar *code = language->code;
		gsize length = strlen (code);

		if (g_ascii_strcasecmp (language_code, code) == 0)
		{
			return language;
		}

		if (use_closest_match &&
		    g_ascii_strncasecmp (language_code, code, length) == 0)
		{
			closest_match = language;
		}
	}

	return closest_match;
}

/**
 * gspell_language_lookup:
 * @language_code: a language code.
 *
 * Returns: (nullable): a #GspellLanguage corresponding to @language_code, or
 * %NULL if not found.
 */
const GspellLanguage *
gspell_language_lookup (const gchar *language_code)
{
	return lookup_language_internal (language_code, TRUE);
}

/**
 * gspell_language_get_code:
 * @language: a #GspellLanguage.
 *
 * Returns: the @language code, for example fr_BE.
 */
const gchar *
gspell_language_get_code (const GspellLanguage *language)
{
	g_return_val_if_fail (language != NULL, NULL);

	return language->code;
}

/**
 * gspell_language_get_name:
 * @language: a #GspellLanguage.
 *
 * Returns the @language name translated to the current locale. For example
 * "French (Belgium)" is returned if the current locale is in English and the
 * @language code is fr_BE.
 *
 * Returns: the @language name.
 */
const gchar *
gspell_language_get_name (const GspellLanguage *language)
{
	g_return_val_if_fail (language != NULL, NULL);

	return language->name;
}

/**
 * gspell_language_compare:
 * @language_a: a #GspellLanguage.
 * @language_b: another #GspellLanguage.
 *
 * Compares alphabetically two languages by their name, as returned by
 * gspell_language_get_name().
 *
 * Returns: an integer less than, equal to, or greater than zero, if @language_a
 * is <, == or > than @language_b.
 */
gint
gspell_language_compare (const GspellLanguage *language_a,
                         const GspellLanguage *language_b)
{
	g_return_val_if_fail (language_a != NULL, 0);
	g_return_val_if_fail (language_b != NULL, 0);

	return g_strcmp0 (language_a->collate_key, language_b->collate_key);
}

/**
 * gspell_language_copy:
 * @language: a #GspellLanguage.
 *
 * Used by language bindings.
 *
 * Returns: a copy of @lang.
 */
GspellLanguage *
gspell_language_copy (const GspellLanguage *language)
{
	g_return_val_if_fail (language != NULL, NULL);

	return (GspellLanguage *) language;
}

/**
 * gspell_language_free:
 * @language: a #GspellLanguage.
 *
 * Used by language bindings.
 */
void
gspell_language_free (GspellLanguage *language)
{
	g_return_if_fail (language != NULL);
}

/**
 * gspell_language_get_name_from_code:
 * @language_code: a language code.
 *
 * Returns: a @name corresponding to @language_code which is supported
 * by the dictionaries and spell checkers found by Enchant, or
 * `unknown (@language_code)` if not found. The returned string should
 * be freed after use.
 */
gchar *
gspell_language_get_name_from_code (const gchar *language_code)
{
	GspellLanguage *lang = lookup_language_internal (language_code, FALSE);

	return lang != NULL ?
	               g_strdup (lang->name) :
	               g_strdup_printf ("unknown (%s)", language_code);
}

/* ex:set ts=8 noet: */
