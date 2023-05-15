
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "converter.c"

/* TODO: refactor code */


typedef int (*Compare_func)(void *, void*);

struct Stack
{
    void *head;
    void *tail;
    int size;
    size_t data_size;
};

struct Stack *create_new_stack(size_t data_size, int default_size)
{
    struct Stack *stack = malloc(sizeof(struct Stack));

    stack->data_size = data_size;

    stack->size = 0;

    stack->head = malloc(sizeof(data_size) * default_size);
    stack->tail = stack->head;
}

void free_stack(struct Stack *stack)
{
    free(stack->head);
    free(stack);
}

void *get_value_from_stack(struct Stack *stack, int index)
{
    if(stack->size)
    {
        unsigned char *ptr;
        unsigned char *tail = (unsigned char*)stack->tail;

        ptr = tail - ((index + 1) * stack->data_size);

        return (void *)ptr;
    }

    return NULL;
}

void *pop_out_the_stack(struct Stack *stack)
{
    if(stack->size)
    {
        unsigned char *ptr;
        unsigned char *tail = (unsigned char*)stack->tail;

        ptr = tail - stack->data_size;

        stack->tail = (void*)ptr;
        stack->size--;
    }

    return stack->tail;
}

void empty_stack(struct Stack *stack)
{
    while (stack->size > 0)
    {
        pop_out_the_stack(stack);
    }  
}

void push_into_stack(void *source, struct Stack *stack)
{
    unsigned char *ptr_source = (unsigned char*)source;
    unsigned char *tail = (unsigned char*)stack->tail;

    for(int i = 0; i < stack->data_size; i++)
    {
        tail[i] = ptr_source[i];
    }

    stack->tail += stack->data_size;
    stack->size++;
}

/***********************************************************************/


enum TAG_FLAG { BEGIN_TAG, HAS_BEGUN, END_NAME_TAG, END_TAG, CLOSURE_SYMBOLE, NULL_TAG_FLAG };

int update_the_tag_flag_according_to_the_current_char(char c, enum TAG_FLAG *flag)
{
    switch (c) //no need to a stack only anflag
    {
    case '<': 
        *flag = BEGIN_TAG;
        break;

    case '>': 
        *flag = END_TAG;
        break;

    case '/': 
        if ( *flag == BEGIN_TAG ) *flag = CLOSURE_SYMBOLE;
        break;

    case ' ': 
        if ( *flag == HAS_BEGUN ) *flag = END_NAME_TAG;
        break;
    
    default:
        if ( *flag == BEGIN_TAG || *flag == HAS_BEGUN ) *flag = HAS_BEGUN; 

        if ( *flag == CLOSURE_SYMBOLE ) *flag = HAS_BEGUN; 

        if ( *flag == END_TAG || *flag == NULL_TAG_FLAG ) *flag = NULL_TAG_FLAG;
    }
}

enum BUFFER_STATE { BUFF_ON, BUFF_DISCARD, BUFF_PENDING, BUFF_OFF };

struct Buffer
{
    char data[1000]; //is it use for <p>???
    int ptr;
    enum BUFFER_STATE state;
};

struct Buffer create_new_buffer()
{
    struct Buffer buffer;

    memset(buffer.data, '\0', sizeof(buffer.data));
    buffer.ptr = 0;
    buffer.state = BUFF_OFF;

    return buffer;
}

void reset_buffer_data(struct Buffer *buffer)
{
    memset(buffer->data, '\0', sizeof(buffer->data));
    buffer->ptr = 0;
}

void update_the_state_of_the_label_tag_buffer_according_to_the_state_of_tag_flag(enum TAG_FLAG flag, enum BUFFER_STATE *state)
{
    switch (flag)
    {
    case BEGIN_TAG:
    case HAS_BEGUN:
        *state = BUFF_ON;
        break;

    case END_TAG:
        if (*state == BUFF_ON ) *state = BUFF_DISCARD;
        break;

    case END_NAME_TAG:
        if (*state == BUFF_ON ) *state = BUFF_DISCARD;
        break;
    
    case CLOSURE_SYMBOLE:
        *state = BUFF_PENDING;
        break;

    default:
        if ( *state == BUFF_PENDING ) *state = BUFF_ON;

        else *state = BUFF_OFF;
    }
}

enum TAG update_the_label_tag_buffer_according_to_his_current_state(char c, struct Buffer *buffer)
{
    enum TAG discard_tag = NULL_TAG;

    switch (buffer->state)
    {
    case BUFF_ON:
        if ( c == '<' || c == '>' || c == ' ' ) return NULL_TAG;
        buffer->data[buffer->ptr] = c;
        buffer->ptr++;
        break;

    case BUFF_DISCARD:
        discard_tag = convert_string_tag_into_enum_tag(buffer->data);
        reset_buffer_data(buffer);
        break;
    
    default:
        break;
    }

    return discard_tag;
}

int analyse_the_tag_discarded_to_trigger_begin_evaluation_document(enum TAG current_tag, enum TAG begin_tag, int *begin_flag)
{
    if( current_tag == begin_tag ) { *begin_flag = 1; return 1; }
    return 0;
}

//problem update inst check stack
void update_closure_tag_flag_according_to_the_tag_flag(enum TAG_FLAG tag_flag, int *closure_tag_flag)
{
    switch (tag_flag)
    {
    case CLOSURE_SYMBOLE:
        *closure_tag_flag = 1;
        break;

    case END_TAG:
        if ( *closure_tag_flag == 1 ) *closure_tag_flag = 0;
        break;
    
    default:
        if ( *closure_tag_flag == 0 ) *closure_tag_flag = -1;
        break;
    }
}

void update_the_current_level_according_to_the_tag_and_the_closure_flag(enum TAG tag, int *current_level, int closure_tag_flag)
{
    switch (tag)
    {
    case BODY:
    case MAIN:
    case DIV:
    case SECTION:
    case ARTICLE:
    case HEADER:
    case UL:
    case LI:
    case OL:
    case A:
    case TABLE:
    case TR:
    case TH:
        if ( closure_tag_flag == -1 ) (*current_level)++;
        else (*current_level)--;
        break;
    
    default:
        break;
    }
}

void push_into_the_stack_tag(enum TAG tag, struct Stack *stack)
{
    push_into_stack(&tag, stack);
}

void dicard_the_tag_stack_until_a_match_with_the_current_tag_was_found(enum TAG tag_to_match, struct Stack *stack)
{
    enum TAG current_value = NULL_TAG;

    while ( current_value != tag_to_match )
    {
        if(stack->size == 0) break;

       current_value  = *(enum TAG *)pop_out_the_stack(stack); //remove all tags until a match is found
    }
}

enum Level_flag { LOCAL_MIN, LOCAL_MAX, SAME_LEVEL, NULL_LEVEL_FLAG };
enum Level_cursor_flag {  MAX_INCREASE, MIN_DECREASE, NULL_CURSOR_FLAG };

struct Level
{
    int current;
    int min_cursor;
    int max_cursor;
    int change;
};

struct Level create_new_level()
{
    struct Level level;

    level.current = 0;
    level.min_cursor = 0;
    level.max_cursor = 0;
    level.change = -1;

    return level;
}

enum Level_cursor_flag update_the_cursor(struct Level *level, enum TAG current_tag)
{
    enum Level_cursor_flag flag = NULL_CURSOR_FLAG;

    if ( current_tag == A && level->max_cursor != level->current )
    {
        flag = MAX_INCREASE;
        level->max_cursor = level->current;
        level->min_cursor = level->current;
        level->change = 0;
    }

    else if ( current_tag == A && level->max_cursor == level->current )
    {
        level->change = 1;
    }

    else if ( level->min_cursor > level->current && level->change < 1 )
    {
        level->min_cursor = level->current;
    }

    else if ( level->min_cursor > level->current && level->change == 1 )
    {
        flag = MIN_DECREASE;
        level->min_cursor = level->current;
        level->max_cursor = level->current;
        level->change = -1;
    }

    return flag;
}

void update_the_stack_group_level_according_to_the_cursor_flag(enum Level_cursor_flag flag, struct Stack *stack, int *group_id)
{
    switch (flag)
    {
    case MIN_DECREASE:
        pop_out_the_stack(stack);
        break;

    case MAX_INCREASE:
        (*group_id)++;
        pop_out_the_stack(stack);
        push_into_stack(group_id, stack);
        break;
    
    default:
        break;
    }
}

int get_the_current_group_id(struct Stack *stack)
{
    int *result = (int *)get_value_from_stack(stack,0);
    if(result) return *result;
    return 0;
}

enum TAG get_the_current_tag(struct Stack *tag_stack)
{
    enum TAG *result = (enum TAG *)get_value_from_stack(tag_stack, 0);
    if(result) return *result;
    return NULL_TAG;
}

struct Handle_A_tag_data
{
    int is_gathering_url;
    char buffer_url[500];
    int i_url;
    int ptr;

    char buffer_text[500];
    int i_text;
    int is_gathering_text;

    int group_level_id;
};

struct Handle_A_tag_data create_A_tag_handler()
{
    struct Handle_A_tag_data data;

    data.is_gathering_url = 0;
    memset(data.buffer_url, '\0', sizeof(data.buffer_url));
    data.i_url = 0;
    data.ptr = 0;

    memset(data.buffer_text, '\0', sizeof(data.buffer_text));
    data.i_text = 0;
    data.is_gathering_text = 0;

    data.group_level_id = 0;

    return data;
}

int search_for_a_word(char c, char *word, int *ptr)
{
    if ( c == word[*ptr] ) (*ptr)++;

    else *ptr = 0;

    if ( *ptr >= strlen(word) ) { *ptr = 0; return 1; }

    return 0;
}

int handle_A_tag(char c, struct Handle_A_tag_data *data, int group_level_id, enum TAG_FLAG flag) //if current tag is A
{
    data->group_level_id = group_level_id;
    //gathering for href
    int result = search_for_a_word( c, "href=\"", &(data->ptr) );

    if ( data->is_gathering_url == 1 && c == '"' ) { data->is_gathering_url = 0; }
    if ( result == 1 ) {data->is_gathering_url = 1; }
    if ( data->is_gathering_url == 1 && c != '"' ) { data->buffer_url[data->i_url] = c; data->i_url++; }

    //gathering for text
    if(flag == BEGIN_TAG || flag == HAS_BEGUN &&  data->is_gathering_text == 1) { data->is_gathering_text = 0; return 1; }
    if( data->is_gathering_text == 1 ) { data->buffer_text[data->i_text] = c; data->i_text++; }
    if(flag == END_TAG && data->i_url++ > 0) data->is_gathering_text = 1;

    return 0;
}

void reset_A_tag_handler(struct Handle_A_tag_data *data)
{
    data->is_gathering_url = 0;
    memset(data->buffer_url, '\0', sizeof(data->buffer_url));
    data->i_url = 0;
    data->ptr = 0;

    memset(data->buffer_text, '\0', sizeof(data->buffer_text));
    data->i_text = 0;
    data->is_gathering_text = 0;

    data->group_level_id = 0;
}

void store_A_tag(struct Handle_A_tag_data *data, FILE *store_file, int current_group_level_id_in_store)
{
    char num[5];
    sprintf(num,"%d",data->group_level_id);
    if(current_group_level_id_in_store != data->group_level_id && current_group_level_id_in_store != 0)
        fprintf(store_file, "\n%s*%s*%s\n", num, data->buffer_text, data->buffer_url);
    else 
        fprintf(store_file, "%s*%s*%s\n", num, data->buffer_text, data->buffer_url);
}

/* TODO: refactor code */

void parse_html(FILE *html_file, FILE *store_file)
{
    fseek(html_file, 0, SEEK_SET);

    char c;

    enum TAG tag_begin = BODY;
    int max_group_level_id = 0;
    enum TAG current_tag = NULL_TAG;

    int begin_flag = 0;
    int closure_tag_flag = -1;
    enum TAG_FLAG tag_flag = NULL_TAG_FLAG;

    struct Buffer buffer_label_tag = create_new_buffer();
    struct Stack *group_level_stack = create_new_stack(sizeof(int), 1000);
    struct Stack *tag_stack = create_new_stack(sizeof(int), 1000);
    struct Handle_A_tag_data data = create_A_tag_handler();

    int current_group_level_id_store = 0; //use to format the storage

    struct Level level = create_new_level();

    while (1)
    {
        /* TODO: refactor code */
        
        c = fgetc(html_file);

        if ( c == EOF ) break;

        update_the_tag_flag_according_to_the_current_char(c, &tag_flag);

        update_the_state_of_the_label_tag_buffer_according_to_the_state_of_tag_flag(tag_flag, &buffer_label_tag.state);

        enum TAG tag_discarded = update_the_label_tag_buffer_according_to_his_current_state(c, &buffer_label_tag);

        if( !begin_flag )
        {
            int result = analyse_the_tag_discarded_to_trigger_begin_evaluation_document(tag_discarded, tag_begin, &begin_flag);

            if ( !result ) continue;
        } 

        update_closure_tag_flag_according_to_the_tag_flag(tag_flag, &closure_tag_flag);

        if ( tag_discarded != NULL_TAG )
        {
            if ( closure_tag_flag == 0 ) dicard_the_tag_stack_until_a_match_with_the_current_tag_was_found(tag_discarded, tag_stack);

            else if( closure_tag_flag == -1 ) push_into_the_stack_tag(tag_discarded, tag_stack);

            update_the_current_level_according_to_the_tag_and_the_closure_flag(tag_discarded, &(level.current), closure_tag_flag);

            current_tag = get_the_current_tag(tag_stack);

            enum Level_cursor_flag level_cursor_flag = update_the_cursor(&level, current_tag);
            
            update_the_stack_group_level_according_to_the_cursor_flag(level_cursor_flag, group_level_stack, &max_group_level_id);
        } 

        if ( current_tag == A )
        {
            int current_group_level_id = get_the_current_group_id(group_level_stack);

            int is_done = handle_A_tag(c, &data, current_group_level_id, tag_flag);

            if( is_done )
            {
                 store_A_tag(&data, store_file, current_group_level_id_store);

                 reset_A_tag_handler(&data);

                 current_group_level_id_store = current_group_level_id;
            }
        } 
    }
}

int main(int argc, char const *argv[])
{
    FILE *html_file = fopen("/data/proc/std/in/stdin","r");
    //FILE *html_file = fopen("test.html","r");
    FILE *store_file = fopen("store.txt","w");

    parse_html(html_file, store_file);

    fclose(html_file);
    fclose(store_file);

    printf("\n");

    return 0;
}
