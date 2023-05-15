
#include <string.h>

enum TAG { DIV, HEADER, ARTICLE, SECTION, MAIN, UL, LI, OL, A, H1, H2, H3, H4, IMG, P, SPAN, I, BODY, TABLE, TR, TH, NULL_TAG };

enum TAG convert_string_tag_into_enum_tag(char *tag_name)
{
    enum TAG flag = NULL_TAG;

    if ( strcmp("div", tag_name) == 0 ) {flag = DIV; return flag;}

    if ( strcmp("header", tag_name) == 0 ) {flag = HEADER; return flag;}

    if ( strcmp("article", tag_name) == 0 ) {flag = ARTICLE; return flag;}

    if ( strcmp("section", tag_name) == 0 ) {flag = SECTION; return flag;}

    if ( strcmp("main", tag_name) == 0 ) {flag = MAIN; return flag;}

    if ( strcmp("ul", tag_name) == 0 ) {flag = UL; return flag;}

    if ( strcmp("li", tag_name) == 0 ) {flag = LI; return flag;}

    if ( strcmp("ol", tag_name) == 0 ) {flag = OL; return flag;}

    if ( strcmp("table", tag_name) == 0 ) {flag = DIV; return flag;}

    if ( strcmp("tr", tag_name) == 0 ) {flag = DIV; return flag;}

    if ( strcmp("th", tag_name) == 0 ) {flag = DIV; return flag;}
    
    if ( strcmp("img", tag_name) == 0 ) {flag = DIV; return flag;}

    if ( strcmp("h1", tag_name) == 0 ) {flag = H1; return flag;}

    if ( strcmp("h2", tag_name) == 0 ) {flag = H2; return flag;}

    if ( strcmp("h3", tag_name) == 0 ) {flag = H3; return flag;}

    if ( strcmp("h4", tag_name) == 0 ) {flag = H4; return flag;}

    if ( strcmp("a", tag_name) == 0 ) {flag = A; return flag;}

    if ( strcmp("p", tag_name) == 0 ) {flag = P; return flag;}

    if ( strcmp("span", tag_name) == 0 ) {flag = SPAN; return flag;}

    if ( strcmp("i", tag_name) == 0 ) {flag = I; return flag;}

    if ( strcmp("body", tag_name) == 0 ) {flag = BODY; return flag;}    
}