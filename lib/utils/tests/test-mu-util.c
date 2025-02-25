/*
** Copyright (C) 2008-2013 Dirk-Jan C. Binnema <djcb@djcbsoftware.nl>
**
** This program is free software; you can redistribute it and/or modify it
** under the terms of the GNU General Public License as published by the
** Free Software Foundation; either version 3, or (at your option) any
** later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software Foundation,
** Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
**
*/

#if HAVE_CONFIG_H
#include "config.h"
#endif /*HAVE_CONFIG_H*/

#include <glib.h>
#include <glib/gstdio.h>

#include <stdlib.h>
#include <unistd.h>
#include <limits.h>

#include "mu-util.h"

static void
test_mu_util_dir_expand_00(void)
{
#ifdef HAVE_WORDEXP_H
	gchar *got, *expected;

	got      = mu_util_dir_expand("~/IProbablyDoNotExist");
	expected = g_strdup_printf("%s%cIProbablyDoNotExist",
				   getenv("HOME"), G_DIR_SEPARATOR);

	g_assert_cmpstr(got, ==, expected);

	g_free(got);
	g_free(expected);
#endif /*HAVE_WORDEXP_H*/
}

static void
test_mu_util_dir_expand_01(void)
{
	/* XXXX: the testcase does not work when using some dir
	 * setups; (see issue #585), although the code should still
	 * work. Turn of the test for now */
	return;

#ifdef HAVE_WORDEXP_H
	{
		gchar *got, *expected;

		got      = mu_util_dir_expand("~/Desktop");
		expected = g_strdup_printf("%s%cDesktop",
					   getenv("HOME"), G_DIR_SEPARATOR);

		g_assert_cmpstr(got, ==, expected);

		g_free(got);
		g_free(expected);
	}
#endif /*HAVE_WORDEXP_H*/
}

static void
test_mu_util_guess_maildir_01(void)
{
	char*       got;
	const char* expected;

	/* skip the test if there's no /tmp */
	if (access("/tmp", F_OK))
		return;

	g_setenv("MAILDIR", "/tmp", TRUE);

	got      = mu_util_guess_maildir();
	expected = "/tmp";

	g_assert_cmpstr(got, ==, expected);
	g_free(got);
}

static void
test_mu_util_guess_maildir_02(void)
{
	char *got, *mdir;

	g_unsetenv("MAILDIR");

	mdir = g_strdup_printf("%s%cMaildir",
			       getenv("HOME"), G_DIR_SEPARATOR);
	got  = mu_util_guess_maildir();

	if (access(mdir, F_OK) == 0)
		g_assert_cmpstr(got, ==, mdir);
	else
		g_assert_cmpstr(got, ==, NULL);

	g_free(got);
	g_free(mdir);
}

static void
test_mu_util_check_dir_01(void)
{
	if (g_access("/usr/bin", F_OK) == 0) {
		g_assert_cmpuint(
		    mu_util_check_dir("/usr/bin", TRUE, FALSE) == TRUE,
		    ==,
		    g_access("/usr/bin", R_OK) == 0);
	}
}

static void
test_mu_util_check_dir_02(void)
{
	if (g_access("/tmp", F_OK) == 0) {
		g_assert_cmpuint(
		    mu_util_check_dir("/tmp", FALSE, TRUE) == TRUE,
		    ==,
		    g_access("/tmp", W_OK) == 0);
	}
}

static void
test_mu_util_check_dir_03(void)
{
	if (g_access(".", F_OK) == 0) {
		g_assert_cmpuint(
		    mu_util_check_dir(".", TRUE, TRUE) == TRUE,
		    ==,
		    g_access(".", W_OK | R_OK) == 0);
	}
}

static void
test_mu_util_check_dir_04(void)
{
	/* not a dir, so it must be false */
	g_assert_cmpuint(
	    mu_util_check_dir("test-util.c", TRUE, TRUE),
	    ==,
	    FALSE);
}

static void
test_mu_util_get_dtype_with_lstat(void)
{
	g_assert_cmpuint(
	    mu_util_get_dtype(MU_TESTMAILDIR, TRUE), ==, DT_DIR);
	g_assert_cmpuint(
	    mu_util_get_dtype(MU_TESTMAILDIR2, TRUE), ==, DT_DIR);
	g_assert_cmpuint(
	    mu_util_get_dtype(MU_TESTMAILDIR2 "/Foo/cur/mail5", TRUE),
	    ==, DT_REG);
}

static void
test_mu_util_supports(void)
{
	gboolean has_guile;
	gchar*   path;

#ifdef BUILD_GUILE
	has_guile = TRUE;
#else
	has_guile = FALSE;
#endif /*BUILD_GUILE*/

	g_assert_cmpuint(mu_util_supports(MU_FEATURE_GUILE), ==, has_guile);

	path = g_find_program_in_path("gnuplot");
	g_free(path);

	g_assert_cmpuint(mu_util_supports(MU_FEATURE_GNUPLOT), ==,
			 path ? TRUE : FALSE);

	g_assert_cmpuint(
	    mu_util_supports(MU_FEATURE_GNUPLOT | MU_FEATURE_GUILE),
	    ==,
	    has_guile && path ? TRUE : FALSE);
}

static void
test_mu_util_program_in_path(void)
{
	g_assert_cmpuint(mu_util_program_in_path("ls"), ==, TRUE);
}


static void
test_mu_util_summarize(void)
{
	const char *txt =
		"Khiron was fortified and made the seat of a pargana during "
		"the reign of Asaf-ud-Daula.\n\the headquarters had previously "
		"been at Satanpur since its foundation and fortification by "
		"the Bais raja Sathna.\n\nKhiron was also historically the seat "
		"of a taluqdari estate belonging to a Janwar dynasty.\n"
		"There were also several Kayasth qanungo families, "
		"including many descended from Rai Sahib Rai, who had been "
		"a chakladar under the Nawabs of Awadh.";

	char *summ = mu_str_summarize(txt, 3);
	g_assert_cmpstr(summ, ==,
			"Khiron was fortified and made the seat of a pargana "
			"during the reign of Asaf-ud-Daula. he headquarters had "
			"previously been at Satanpur since its foundation and "
			"fortification by the Bais raja Sathna. ");
	g_free (summ);
}


static void
test_mu_error(void)
{
	GQuark q;
	GError *err;
	gboolean res;

	q = mu_util_error_quark();
	g_assert_true(q != 0);


	err = NULL;
	res = mu_util_g_set_error(&err, MU_ERROR_IN_PARAMETERS,
				  "Hello, %s!", "World");

	g_assert_false(res);
	g_assert_cmpuint(err->domain, ==, q);
	g_assert_cmpuint(err->code, ==, MU_ERROR_IN_PARAMETERS);
	g_assert_cmpstr(err->message,==,"Hello, World!");

	g_clear_error(&err);
}


int
main(int argc, char* argv[])
{
	g_test_init(&argc, &argv, NULL);

	/* mu_util_dir_expand */
	g_test_add_func("/mu-util/mu-util-dir-expand-00",
			test_mu_util_dir_expand_00);
	g_test_add_func("/mu-util/mu-util-dir-expand-01",
			test_mu_util_dir_expand_01);

	/* mu_util_guess_maildir */
	g_test_add_func("/mu-util/mu-util-guess-maildir-01",
			test_mu_util_guess_maildir_01);
	g_test_add_func("/mu-util/mu-util-guess-maildir-02",
			test_mu_util_guess_maildir_02);

	/* mu_util_check_dir */
	g_test_add_func("/mu-util/mu-util-check-dir-01",
			test_mu_util_check_dir_01);
	g_test_add_func("/mu-util/mu-util-check-dir-02",
			test_mu_util_check_dir_02);
	g_test_add_func("/mu-util/mu-util-check-dir-03",
			test_mu_util_check_dir_03);
	g_test_add_func("/mu-util/mu-util-check-dir-04",
			test_mu_util_check_dir_04);

	g_test_add_func("/mu-util/mu-util-get-dtype-with-lstat",
			test_mu_util_get_dtype_with_lstat);

	g_test_add_func("/mu-util/mu-util-supports", test_mu_util_supports);
	g_test_add_func("/mu-util/mu-util-program-in-path",
			test_mu_util_program_in_path);

	g_test_add_func("/mu-util/summarize", test_mu_util_summarize);
	g_test_add_func("/mu-util/error", test_mu_error);

	return g_test_run();
}
