/*
 * blogc: A blog compiler.
 * Copyright (C) 2015 Rafael G. Martins <rafael@rafaelmartins.eng.br>
 *
 * This program can be distributed under the terms of the BSD License.
 * See the file COPYING.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <string.h>
#include "../src/source-parser.h"
#include "../src/error.h"
#include "../src/utils/utils.h"


static void
test_source_parse(void **state)
{
    const char *a =
        "VAR1: asd asd\n"
        "VAR2: 123chunda\n"
        "----------\n"
        "# This is a test\n"
        "\n"
        "bola\n";
    blogc_error_t *err = NULL;
    b_trie_t *source = blogc_source_parse(a, strlen(a), &err);
    assert_null(err);
    assert_non_null(source);
    assert_int_equal(b_trie_size(source), 3);
    assert_string_equal(b_trie_lookup(source, "VAR1"), "asd asd");
    assert_string_equal(b_trie_lookup(source, "VAR2"), "123chunda");
    assert_string_equal(b_trie_lookup(source, "CONTENT"),
        "# This is a test\n"
        "\n"
        "bola\n");
    b_trie_free(source);
}


static void
test_source_parse_with_spaces(void **state)
{
    const char *a =
        "\n  \n"
        "VAR1: chunda     \t   \n"
        "\n\n"
        "BOLA: guda\n"
        "----------\n"
        "# This is a test\n"
        "\n"
        "bola\n";
    blogc_error_t *err = NULL;
    b_trie_t *source = blogc_source_parse(a, strlen(a), &err);
    assert_null(err);
    assert_non_null(source);
    assert_int_equal(b_trie_size(source), 3);
    assert_string_equal(b_trie_lookup(source, "VAR1"), "chunda");
    assert_string_equal(b_trie_lookup(source, "BOLA"), "guda");
    assert_string_equal(b_trie_lookup(source, "CONTENT"),
        "# This is a test\n"
        "\n"
        "bola\n");
    b_trie_free(source);
}


int
main(void)
{
    const UnitTest tests[] = {
        unit_test(test_source_parse),
        unit_test(test_source_parse_with_spaces),
    };
    return run_tests(tests);
}
