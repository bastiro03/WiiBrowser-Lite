#include "parser.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

int main(void)
{
    const char *fixtures[] = {"p { color: red; }", "a { display: none; }", "body { margin: 0; color: black; }"};
    for (unsigned i = 0; i < sizeof(fixtures) / sizeof(fixtures[0]); ++i) {
        struct selector_list_t *rules = css_parse(fixtures[i], (int)strlen(fixtures[i]));
        assert(rules != NULL && rules->selector != NULL);
        free_rulesets(rules);
    }
    puts("Generated CSS parser/lexer compile and sanitizer smoke tests passed.");
    return 0;
}
