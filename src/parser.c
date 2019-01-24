//
//  igs_parser.c
//
//  Created by Stephane Vales
//  Modified by Patxi Berard
//  Modified by Vincent Deliencourt
//  Copyright © 2017 Ingenuity i/o. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#if (defined WIN32 || defined _WIN32)
#include "unixfunctions.h"
#endif

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdocumentation"
#include "yajl/yajl_tree.h"
#pragma clang diagnostic pop
#include "yajl/yajl_gen.h"
#include "ingescape_private.h"

#define STR_CATEGORY "category"
#define STR_DEFINITION "definition"
#define STR_NAME "name"
#define STR_DESCRIPTION "description"
#define STR_VERSION "version"
#define STR_PARAMETERS "parameters"
#define STR_OUTPUTS "outputs"
#define STR_INPUTS "inputs"
#define STR_TOKENS "tokens"
#define STR_ARGUMENTS "arguments"
#define STR_REPLY "reply"
#define STR_CATEGORIES "categories"
#define STR_TYPE "type"
#define STR_VALUE "value"

#define DEF_NO_NAME "NO_NAME"
#define DEF_NO_DESCRIPTION "NO_DESCRIPTION"
#define DEF_NO_VERSION "NO_VERSION"

#define MAP_NO_NAME "NO_NAME"
#define MAP_NO_DESCRIPTION "NO_DESCRIPTION"
#define MAP_NO_VERSION "NO_VERSION"

char definitionPath[MAX_PATH] = "";

iopType_t string_to_value_type(const char* str) {
    
    if (str != NULL){
        if (!strcmp(str, "INTEGER"))
            return IGS_INTEGER_T;
        if (!strcmp(str, "DOUBLE"))
            return IGS_DOUBLE_T;
        if (!strcmp(str, "STRING"))
            return IGS_STRING_T;
        if (!strcmp(str, "BOOL"))
            return IGS_BOOL_T;
        if (!strcmp(str, "IMPULSION"))
            return IGS_IMPULSION_T;
        if (!strcmp(str, "DATA"))
            return IGS_DATA_T;
    }
    
    igs_error("unknown value type \"%s\" to convert", str);
    return IGS_UNKNOWN_T;
}

bool string_to_boolean(const char* str) {
    if(str != NULL){
        if (!strcmp(str, "true"))
            return true;
        
        if (!strcmp(str, "false"))
            return false;
    }
    
    igs_warn("unknown string \"%s\" to convert", str);
    return false;
}

const char* value_type_to_string (iopType_t type) {
    switch (type) {
        case IGS_INTEGER_T:
            return "INTEGER";
            break;
        case IGS_DOUBLE_T:
            return "DOUBLE";
            break;
        case IGS_STRING_T:
            return "STRING";
            break;
        case IGS_BOOL_T:
            return "BOOL";
            break;
        case IGS_IMPULSION_T:
            return "IMPULSION";
            break;
        case IGS_DATA_T:
            return "DATA";
            break;
        case IGS_UNKNOWN_T:
            return "UNKNOWN";
            break;
        default:
            igs_error("unknown iopType_t to convert");
            break;
    }
    
    return "";
}

const char* boolean_to_string (bool boolean) {
    return (boolean ? "true" : "false");
}

////////////////////////////////////////
// IOP parsing

//parse an agent_iop_t data and add it to the corresponding hash table
static void json_add_iop_to_hash (agent_iop_t **hasht, iop_t type,
                                   yajl_val obj){
    const char *name = NULL;
    iopType_t valType = IGS_UNKNOWN_T;
    yajl_val value = NULL;

    if (YAJL_IS_OBJECT(obj)){
        size_t nb = obj->u.object.len;
        for (size_t i = 0; i < nb; i++){
            const char *key = obj->u.object.keys[i];
            if (strcmp("name", key) == 0){
                name = YAJL_GET_STRING(obj->u.object.values[i]);
            }else if (strcmp("type", key) == 0){
                valType = string_to_value_type (YAJL_GET_STRING(obj->u.object.values[i]));
                if (valType == IGS_UNKNOWN_T){
                    return;
                }
            }else if (strcmp("value", key) == 0){
                value = obj->u.object.values[i];
            }
        }
    }
    
    agent_iop_t *iop = NULL;
    if (name != NULL){
        //handle name, value type and value
        char *n = strndup(name, MAX_IOP_NAME_LENGTH);
        bool spaceInName = false;
        size_t lengthOfN = strlen(n);
        for (size_t i = 0; i < lengthOfN; i++){
            if (n[i] == ' '){
                n[i] = '_';
                spaceInName = true;
            }
        }
        if (spaceInName){
            igs_warn("Spaces are not allowed in IOP name: %s has been renamed to %s", name, n);
        }
        HASH_FIND_STR(*hasht, n, iop);
        if (iop == NULL){
            iop = calloc (1, sizeof (agent_iop_t));
            iop->name = n;
            iop->value_type = valType;
            switch (iop->value_type) {
                case IGS_INTEGER_T:
                    iop->value.i =(int) YAJL_GET_INTEGER (value);
                    break;
                case IGS_DOUBLE_T:
                    iop->value.d = YAJL_GET_DOUBLE (value);
                    break;
                case IGS_BOOL_T:
                    iop->value.b = string_to_boolean (YAJL_GET_STRING(value));
                    break;
                case IGS_STRING_T:
                    iop->value.s = strdup (YAJL_IS_STRING(value) ? value->u.string : NULL);
                    break;
                case IGS_IMPULSION_T:
                    //IMPULSION has no value
                    break;
                case IGS_DATA_T:
                    //FIXME : we store data as string but we should check it convert it to hexa
                    //data->value.s = strdup (YAJL_IS_STRING(obj->u.object.values[2]) ? obj->u.object.values[2]->u.string : "");
                    break;
                default:
                    igs_warn("unknown data type to load from json for %s", n);
                    break;
            }
            iop->is_muted = false;
            iop->type = type;
            HASH_ADD_STR(*hasht, name, iop);
        }else{
            igs_warn("%s already exists", n);
        }
    }
}

//parse a tab of agent_iop_t data and add them into the corresponding hash table
static void json_add_iops (yajl_val node, const char** path, iop_t type,
                           agent_iop_t **hasht) {
    yajl_val v;
    v = yajl_tree_get(node, path, yajl_t_array);

    if (v && YAJL_IS_ARRAY(v)){
        unsigned int  i;
        for (i = 0; i < v->u.array.len; i++ ){
            yajl_val obj = v->u.array.values[i];
            if( obj && YAJL_IS_OBJECT(obj))
                json_add_iop_to_hash (hasht, type, obj);
        }
    }
}

////////////////////////////////////////
// tokens parsing

static void json_parse_token_arguments (igs_token_t *token, yajl_val arguments){
    if (YAJL_IS_ARRAY(arguments)){
        size_t nbArgs = arguments->u.array.len;
        for (size_t i = 0; i < nbArgs; i++){
            //iterate on arguments
            yajl_val arg = arguments->u.array.values[i];
            
            if (YAJL_IS_OBJECT(arg)){
                size_t nbKeys = arg->u.object.len;
                const char *name = NULL;
                iopType_t valType = IGS_UNKNOWN_T;
                
                for (size_t j = 0; j < nbKeys; j++){
                    //iterate on keys for this argument
                    const char *key = arg->u.object.keys[j];
                    if (strcmp("name", key) == 0){
                        name = YAJL_GET_STRING(arg->u.object.values[j]);
                    }else if (strcmp("type", key) == 0){
                        valType = string_to_value_type (YAJL_GET_STRING(arg->u.object.values[j]));
                    }
                }
                
                if (name != NULL){
                    char *n = strndup(name, MAX_IOP_NAME_LENGTH);
                    bool spaceInName = false;
                    size_t lengthOfN = strlen(n);
                    for (size_t _i = 0; _i < lengthOfN; _i++){
                        if (n[_i] == ' '){
                            n[_i] = '_';
                            spaceInName = true;
                        }
                    }
                    if (spaceInName){
                        igs_warn("Spaces are not allowed in token argument name: %s has been renamed to %s", name, n);
                    }
                    if (valType != IGS_UNKNOWN_T){
                        igs_tokenArgument_t *tokenArg = calloc(1, sizeof(igs_tokenArgument_t));
                        tokenArg->name = n;
                        tokenArg->type = valType;
                        LL_APPEND(token->arguments, tokenArg);
                    }else{
                        free(n);
                    }
                }
            }else{
                igs_error("argument is not passed as an object");
            }
        }
    }else{
        igs_error("arguments are not passed as an array");
    }
}

//parse a token and add it to the corresponding hash table
static void json_add_token_to_hash (igs_token_t **hasht, yajl_val obj){
    
    const char *name = NULL;
    const char *description = NULL;
    yajl_val arguments = NULL;
    yajl_val reply = NULL;
    
    if (YAJL_IS_OBJECT(obj)){
        size_t nb = obj->u.object.len;
        for (size_t i = 0; i < nb; i++){
            const char *key = obj->u.object.keys[i];
            if (strcmp("name", key) == 0){
                name = YAJL_GET_STRING(obj->u.object.values[i]);
            }else if (strcmp("description", key) == 0){
                description = YAJL_GET_STRING(obj->u.object.values[i]);
            }else if (strcmp("arguments", key) == 0){
                arguments = obj->u.object.values[i];
            }else if (strcmp("reply", key) == 0){
                reply = obj->u.object.values[i];
            }
        }
    }
    char *n = NULL;
    if (name != NULL){
        n = strndup(name, MAX_IOP_NAME_LENGTH);
        bool spaceInName = false;
        size_t lengthOfN = strlen(n);
        for (size_t i = 0; i < lengthOfN; i++){
            if (n[i] == ' '){
                n[i] = '_';
                spaceInName = true;
            }
        }
        if (spaceInName){
            igs_warn("Spaces are not allowed in token name: %s has been renamed to %s", name, n);
        }
    }else{
        igs_warn("parsed token with NULL name");
    }
    
    igs_token_t *token = NULL;
    HASH_FIND_STR(*hasht, n, token);
    if (token == NULL){
        token = calloc(1, sizeof(igs_token_t));
        token->name = n;
        if (description != NULL)
            token->description = strndup(description, MAX_DESCRIPTION_LENGTH);
        if (arguments != NULL){
            json_parse_token_arguments(token, arguments);
        }
        //TODO: parse reply
        HASH_ADD_STR(*hasht, name, token);
    }else{
        igs_warn("%s already exists", name);
    }
}

//parse a tab of tokens and add them into the corresponding hash table
static void json_add_tokens (yajl_val node, const char **path, igs_token_t **hasht){
    yajl_val v;
    v = yajl_tree_get(node, path, yajl_t_array);
    
    if (v && YAJL_IS_ARRAY(v)){
        unsigned int  i;
        for (i = 0; i < v->u.array.len; i++ ){
            yajl_val obj = v->u.array.values[i];
            if( obj && YAJL_IS_OBJECT(obj))
                json_add_token_to_hash (hasht, obj);
        }
    }
}

////////////////////////////////////////
// File reading and parsing

// fetch a JSON file and convert it into string readable by the JSON parser
static char* json_fetch (const char* path) {

    FILE *file;
    char buff[BUFSIZ];
    char *js = NULL;
    int jslen = 0;
    unsigned int rd;

    file = fopen(path, "r");
    if (!file){
        igs_error("file %s not found", path);
        return 0;
    }

    /* read the whole config file */
    do {

        rd = (unsigned int) fread(buff, 1, sizeof(buff) , file);
        /* file read error handling */
        if (rd == 0 && !feof(stdin)) {
            igs_error("could not read %s", path);
            return 0;
        }

        /* rebuild the json string */
        js = realloc(js, jslen + rd + 1);
        if (!js) {
            igs_error("could not realloc parsed string");
            return 0;
        }
        strncpy(js + jslen, buff, rd);
        jslen = jslen + rd;

    }while (rd >= sizeof(buff));

    if (file)
        fclose(file);

    return js;
}

// convert JSON string into DOM
static int json_tokenize (const char* json_str, yajl_val *node) {

    char errbuf[BUFSIZ] = "unknown error";
    /* we have the whole config file in memory.  let's parse it ... */
    *node = yajl_tree_parse(json_str, errbuf, sizeof(errbuf));

    /* parse error handling */
    if (!node || strlen(errbuf) > 0) {
        igs_error("could not parse string (%s)", errbuf);
        return 0;
    }

    return 1;
}

// convert a definition json file into a definition structure
static definition* json_parse_definition (yajl_val node) {
    definition *def;
    yajl_val v;
    def = (definition*) calloc(1, sizeof(definition));
    const char * path[] = { STR_DEFINITION, "", (const char *) 0 };

    path[1] = STR_NAME;
    v = yajl_tree_get(node, path, yajl_t_any);
    if (v){
        def->name = strdup (YAJL_IS_STRING(v) ? (v)->u.string : NULL);
    }

    path[1] = STR_DESCRIPTION;
    v = yajl_tree_get(node, path, yajl_t_any);
    if (v){
        def->description = strdup (YAJL_IS_STRING(v) ? (v)->u.string : NULL);
    }

    path[1] = STR_VERSION;
    v = yajl_tree_get(node, path, yajl_t_any);
    if (v){
        def->version = strdup (YAJL_IS_STRING(v) ? (v)->u.string : NULL);
    }

    path[1] = STR_INPUTS;
    json_add_iops (node, path, IGS_INPUT_T, &def->inputs_table);

    path[1] = STR_OUTPUTS;
    json_add_iops (node, path, IGS_OUTPUT_T, &def->outputs_table);

    path[1] = STR_PARAMETERS;
    json_add_iops (node, path, IGS_PARAMETER_T, &def->params_table);
    
    path[1] = STR_TOKENS;
    json_add_tokens (node, path, &def->tokens_table);

//    path[1] = STR_CATEGORIES;
//    v = yajl_tree_get(node, path, yajl_t_array);
//    if (v && YAJL_IS_ARRAY(v)){
//        unsigned int  i;
//        for (i = 0; i < v->u.array.len; i++ ){
//            yajl_val obj = v->u.array.values[i];
//            if( obj && YAJL_IS_OBJECT(obj))
//                json_add_category_to_hash (&def->categories, obj);
//        }
//    }

    return def;
}

// parse a tab of mapping output type and add them into the corresponding hash table
static void json_add_map_out_to_hash (mapping_element_t** hasht,
                                       yajl_val current_map_out){

    const char* input_name = NULL;
    const char* agent_name = NULL;
    const char* output_name = NULL;
    yajl_val v;
    const char * path_in_current[] = { "", (const char *) 0 };

    //input_name
    path_in_current[0] = "input_name";
    v = yajl_tree_get(current_map_out, path_in_current, yajl_t_any);
    if (v){
        input_name = YAJL_GET_STRING(v);
    }
    char *reviewedFromOurInput = strndup(input_name, MAX_IOP_NAME_LENGTH);
    bool spaceInName = false;
    size_t lengthOfReviewedFromOurInput = strlen(reviewedFromOurInput);
    for (size_t i = 0; i < lengthOfReviewedFromOurInput; i++){
        if (reviewedFromOurInput[i] == ' '){
            reviewedFromOurInput[i] = '_';
            spaceInName = true;
        }
    }
    if (spaceInName){
        igs_warn("Mapping parser : spaces are not allowed in IOP: %s has been renamed to %s\n", input_name, reviewedFromOurInput);
    }
    
    //agent_name
    path_in_current[0] = "agent_name";
    v = yajl_tree_get(current_map_out, path_in_current, yajl_t_any);
    if (v){
        agent_name = YAJL_GET_STRING(v);
    }
    char *reviewedToAgent = strndup(agent_name, MAX_IOP_NAME_LENGTH);
    size_t lengthOfReviewedToAgent = strlen(reviewedToAgent);
    spaceInName = false;
    for (size_t i = 0; i < lengthOfReviewedToAgent; i++){
        if (reviewedToAgent[i] == ' '){
            reviewedToAgent[i] = '_';
            spaceInName = true;
        }
    }
    if (spaceInName){
        igs_warn("Mapping parser : spaces are not allowed in agent name: %s has been renamed to %s\n", agent_name, reviewedToAgent);
    }
    
    //output_name
    path_in_current[0] = "output_name";
    v = yajl_tree_get(current_map_out, path_in_current, yajl_t_any);
    if (v){
        output_name = YAJL_GET_STRING(v);
    }
    char *reviewedWithOutput = strndup(output_name, MAX_IOP_NAME_LENGTH);
    size_t lengthOfReviewedWithOutput = strlen(reviewedWithOutput);
    spaceInName = false;
    for (size_t i = 0; i < lengthOfReviewedWithOutput; i++){
        if (reviewedWithOutput[i] == ' '){
            reviewedWithOutput[i] = '_';
            spaceInName = true;
        }
    }
    if (spaceInName){
        igs_warn("Mapping parser : spaces are not allowed in IOP: %s has been renamed to %s\n", output_name, reviewedWithOutput);
    }
    
    unsigned long len = strlen(reviewedFromOurInput)+strlen(reviewedToAgent)+strlen(reviewedWithOutput)+3+1;
    char *mashup = calloc(1, len*sizeof(char));
    strcpy(mashup, reviewedFromOurInput);
    strcat(mashup, ".");//separator
    strcat(mashup, reviewedToAgent);
    strcat(mashup, ".");//separator
    strcat(mashup, reviewedWithOutput);
    mashup[len -1] = '\0';
    unsigned long h = djb2_hash((unsigned char *)mashup);
    free (mashup);
    
    mapping_element_t *tmp = NULL;
    if (*hasht != NULL){
        HASH_FIND(hh, *hasht, &h, sizeof(unsigned long), tmp);
    }
    if (tmp == NULL){
        //element does not exist yet : create and register it
        mapping_element_t *new = mapping_createMappingElement(reviewedFromOurInput, reviewedToAgent, reviewedWithOutput);
        new->id = h;
        HASH_ADD(hh, *hasht, id, sizeof(unsigned long), new);
    }
    free(reviewedFromOurInput);
    free(reviewedToAgent);
    free(reviewedWithOutput);
}

// parse a tab of mapping category type and add them into the corresponding hash table
static void json_add_map_cat_to_hash (mapping_element_t** hasht,
                                       yajl_val current_map_out){
    IGS_UNUSED(hasht)
    IGS_UNUSED(current_map_out)

    return;
//    const char* agent_name;
//    const char* category_name;
//    struct mapping_cat *map_cat = NULL;
//    yajl_val v;
//    const char * path_in_current[] = { "", (const char *) 0 };
//
//    path_in_current[0] = "map_cat_id";
//    v = yajl_tree_get(current_map_out, path_in_current, yajl_t_any);
//    if (v) {
//        /* check if the key already exist */
//        int map_cat_id = (int) YAJL_GET_INTEGER(v);
//
//        HASH_FIND_INT(*hasht, &map_cat_id , map_cat);
//
//        if (map_cat == NULL){
//            map_cat = calloc (1, sizeof (struct mapping_cat));
//            map_cat->map_cat_id = map_cat_id;
//            map_cat->state = OFF;
//
//            //agent_name
//            path_in_current[0] = "agent_name";
//            v = yajl_tree_get(current_map_out, path_in_current, yajl_t_any);
//            if (v){
//                agent_name = YAJL_GET_STRING(v);
//                map_cat->agent_name = strdup (agent_name);
//            }
//
//            //category_name
//            path_in_current[0] = "category_name";
//            v = yajl_tree_get(current_map_out, path_in_current, yajl_t_any);
//            if (v){
//                category_name = YAJL_GET_STRING(v);
//                map_cat->category_name = strdup (category_name);
//            }
//
//            HASH_ADD_INT(*hasht , map_cat_id, map_cat);  /* id: name of key field */
//        }
//    }
}

// convert a map.json file into a mapping (output & category) structure
static mapping_t* json_parse_mapping (yajl_val node) {

    mapping_t* mapp;
    yajl_val v;
    mapp = (mapping_t*) calloc(1, sizeof(mapping_t));
    const char * path[] = { "mapping", "", (const char *) 0 };

    path[1] = STR_NAME;
    v = yajl_tree_get(node, path, yajl_t_any);
    if (v){
        const char* name = YAJL_GET_STRING(v);
        mapp->name = strdup (name);
    }


    path[1] = STR_DESCRIPTION;
    v = yajl_tree_get(node, path, yajl_t_any);
    if (v){
        const char* description = YAJL_GET_STRING(v);
        mapp->description = strdup (description);
    }

    path[1] = STR_VERSION;
    v = yajl_tree_get(node, path, yajl_t_any);
    if (v){
        const char* version = YAJL_GET_STRING(v);
        mapp->version = strdup (version);
    }

    path[1] = "mapping_out";
    v = yajl_tree_get(node, path, yajl_t_array);
    if (v && YAJL_IS_ARRAY(v)){
        unsigned int  i;
        for (i = 0; i < v->u.array.len; i++ ){
            yajl_val obj = v->u.array.values[i];
            if( obj && YAJL_IS_OBJECT(obj))
                json_add_map_out_to_hash (&mapp->map_elements, obj);
        }
    }

    path[1] = "mapping_cat";
    v = yajl_tree_get(node, path, yajl_t_array);
    if (v && YAJL_IS_ARRAY(v)){
        unsigned int  i;
        for (i = 0; i < v->u.array.len; i++ ){
            yajl_val obj = v->u.array.values[i];
            if( obj && YAJL_IS_OBJECT(obj))
                json_add_map_cat_to_hash (&mapp->map_elements, obj);
        }
    }

    return mapp;
}

/////////////////////////
// Dumping functions


// convert a token into json string
static void json_dump_token (yajl_gen *g, igs_token_t *token) {
    
    yajl_gen_map_open(*g);
    
    yajl_gen_string(*g, (const unsigned char *) STR_NAME, strlen(STR_NAME));
    yajl_gen_string(*g, (const unsigned char *) token->name, strlen (token->name));
    
    if (token->description != NULL){
        yajl_gen_string(*g, (const unsigned char *) STR_DESCRIPTION, strlen(STR_DESCRIPTION));
        yajl_gen_string(*g, (const unsigned char *) token->description, strlen (token->description));
    }
    
    igs_tokenArgument_t *arg = NULL;
    int nbArgs = 0;
    DL_COUNT(token->arguments, arg, nbArgs);
    if ((token->arguments != NULL) && (nbArgs > 0)){
        yajl_gen_string(*g, (const unsigned char *) STR_ARGUMENTS, strlen(STR_ARGUMENTS));
        yajl_gen_array_open(*g);
        DL_FOREACH(token->arguments, arg){
            yajl_gen_map_open(*g);
            yajl_gen_string(*g, (const unsigned char *) STR_NAME, strlen(STR_NAME));
            yajl_gen_string(*g, (const unsigned char *) arg->name, strlen(arg->name));
            yajl_gen_string(*g, (const unsigned char *) STR_TYPE, strlen(STR_TYPE));
            const char *type = value_type_to_string(arg->type);
            yajl_gen_string(*g, (const unsigned char *) type, strlen(type));
            yajl_gen_map_close(*g);
        }
        yajl_gen_array_close(*g);
    }
    //TODO: dump reply
    
    yajl_gen_map_close(*g);
}

// convert an agent_iop_t structure into json string
static void json_dump_iop (yajl_gen *g, agent_iop_t* aiop) {
    
    yajl_gen_map_open(*g);
    
    yajl_gen_string(*g, (const unsigned char *) STR_NAME, strlen(STR_NAME));
    yajl_gen_string(*g, (const unsigned char *) aiop->name, strlen (aiop->name));
    
    yajl_gen_string(*g, (const unsigned char *) STR_TYPE, strlen(STR_TYPE));
    yajl_gen_string(*g, (const unsigned char *) value_type_to_string(aiop->value_type), strlen(value_type_to_string(aiop->value_type)));
    
    yajl_gen_string(*g, (const unsigned char *) STR_VALUE, strlen(STR_VALUE));
    
    switch (aiop->value_type) {
        case IGS_INTEGER_T:
            yajl_gen_integer(*g, aiop->value.i);
            break;

        case IGS_DOUBLE_T:
            yajl_gen_double(*g, aiop->value.d);
            break;

        case IGS_BOOL_T:
            yajl_gen_string(*g, (const unsigned char *) boolean_to_string(aiop->value.b), strlen(boolean_to_string(aiop->value.b)));
            break;

        case IGS_STRING_T:
            {
                if (yajl_gen_string(*g, (const unsigned char *) aiop->value.s, strlen(aiop->value.s)) == yajl_gen_invalid_string)
                {
                    igs_warn("Mapping parser : json_dump_iop failed to dump a string value - it may not be a valid UTF8 string - %s\n", aiop->value.s);
                    yajl_gen_string(*g, (const unsigned char *) "", 0);
                }
            }
            break;

        case IGS_IMPULSION_T:
            yajl_gen_string(*g, (const unsigned char *) "", 0);
            break;

        case IGS_DATA_T:
            yajl_gen_string(*g, (const unsigned char *) "", 0);
            break;

        default:
            {
                igs_warn("unknown data type to convert in string (%d)", aiop->value_type);
                yajl_gen_string(*g, (const unsigned char *) "", 0);
            }
            break;
    }
    yajl_gen_map_close(*g);
}

// convert a definition structure into definition.json string
static void json_dump_definition (yajl_gen *g, definition* def) {
    
    unsigned int hashCount = 0;
    agent_iop_t *d;
    
    yajl_gen_map_open(*g);
    
    yajl_gen_string(*g, (const unsigned char *) STR_NAME, strlen(STR_NAME));
    //Get the agent name from the network layer
    if (def->name == NULL){
        yajl_gen_string(*g, (const unsigned char *) DEF_NO_NAME, strlen (DEF_NO_NAME));
    }else{
        yajl_gen_string(*g, (const unsigned char *) def->name, strlen (def->name));
    }
    
    
    yajl_gen_string(*g, (const unsigned char *) STR_DESCRIPTION, strlen(STR_DESCRIPTION));
    if(def->description != NULL){
        yajl_gen_string(*g, (const unsigned char *) def->description, strlen (def->description));
    } else {
        yajl_gen_string(*g, (const unsigned char *) DEF_NO_DESCRIPTION, strlen(DEF_NO_DESCRIPTION));
    }
    
    yajl_gen_string(*g, (const unsigned char *) STR_VERSION, strlen(STR_VERSION));
    if(def->version != NULL){
        yajl_gen_string(*g, (const unsigned char *) def->version, strlen(def->version));
    } else {
        yajl_gen_string(*g, (const unsigned char *) DEF_NO_VERSION, strlen(DEF_NO_VERSION));
    }
    
    hashCount = HASH_COUNT(def->params_table);
    if (hashCount) {
        yajl_gen_string(*g, (const unsigned char *) STR_PARAMETERS, strlen(STR_PARAMETERS));
        yajl_gen_array_open(*g);
        for(d=def->params_table; d != NULL; d=d->hh.next) {
            json_dump_iop (g, d);
        }
        yajl_gen_array_close(*g);
    }
    
    hashCount = HASH_COUNT(def->inputs_table);
    if (hashCount) {
        yajl_gen_string(*g, (const unsigned char *) STR_INPUTS, strlen(STR_INPUTS));
        yajl_gen_array_open(*g);
        for(d=def->inputs_table; d != NULL; d=d->hh.next) {
            json_dump_iop (g, d);
        }
        yajl_gen_array_close(*g);
    }
    
    hashCount = HASH_COUNT(def->outputs_table);
    if (hashCount) {
        yajl_gen_string(*g, (const unsigned char *) STR_OUTPUTS, strlen(STR_OUTPUTS));
        yajl_gen_array_open(*g);
        for(d=def->outputs_table; d != NULL; d=d->hh.next) {
            json_dump_iop (g, d);
        }
        yajl_gen_array_close(*g);
    }
    
    hashCount = HASH_COUNT(def->tokens_table);
    if (hashCount) {
        yajl_gen_string(*g, (const unsigned char *) STR_TOKENS, strlen(STR_TOKENS));
        yajl_gen_array_open(*g);
        igs_token_t *t = NULL, *tmp = NULL;
        HASH_ITER(hh, def->tokens_table, t, tmp){
            json_dump_token (g, t);
        }
        yajl_gen_array_close(*g);
    }
    
//    struct category *cat;
//    hashCount = HASH_COUNT(def->categories);
//    if (hashCount) {
//        yajl_gen_string(*g, (const unsigned char *) STR_CATEGORIES, strlen(STR_CATEGORIES));
//        yajl_gen_array_open(*g);
//        for(cat=def->categories; cat != NULL; cat=cat->hh.next) {
//            json_dump_category(g, cat);
//        }
//        yajl_gen_array_close(*g);
//    }

    yajl_gen_map_close(*g);
}

//convert a mapping_out structure into json string
static void json_dump_mapping_out (yajl_gen *g, mapping_element_t* mapp_out) {

    yajl_gen_map_open(*g);

    yajl_gen_string(*g, (const unsigned char *) "input_name", strlen("input_name"));
    yajl_gen_string(*g, (const unsigned char *) mapp_out->input_name, strlen (mapp_out->input_name));

    yajl_gen_string(*g, (const unsigned char *) "agent_name", strlen("agent_name"));
    yajl_gen_string(*g, (const unsigned char *) mapp_out->agent_name, strlen(mapp_out->agent_name));

    yajl_gen_string(*g, (const unsigned char *) "output_name", strlen("output_name"));
    yajl_gen_string(*g, (const unsigned char *) mapp_out->output_name, strlen(mapp_out->output_name));

    yajl_gen_map_close(*g);
}

/*
 * Function: json_dump_mapping_cat
 * -----------------------
 *   convert a mapping_cat structure into json string
 */

//static void json_dump_mapping_cat (yajl_gen *g, mapping_cat* mapp_cat) {
//
//    yajl_gen_map_open(*g);
//
//    yajl_gen_string(*g, (const unsigned char *) "agent_name", strlen("agent_name"));
//    yajl_gen_string(*g, (const unsigned char *) mapp_cat->agent_name, strlen (mapp_cat->agent_name));
//
//    yajl_gen_string(*g, (const unsigned char *) "category_name", strlen("category_name"));
//    yajl_gen_string(*g, (const unsigned char *) mapp_cat->category_name, strlen(mapp_cat->category_name));
//
//    yajl_gen_map_close(*g);
//}

//convert a mapping structure into mapping.json string
static void json_dump_mapping (yajl_gen *g, mapping_t* mapp) {

    unsigned int hashCount = 0;
    mapping_element_t *currentMapOut = NULL;

    if(mapp != NULL)
    {
        //    struct mapping_cat *currentMapCat = NULL;

        yajl_gen_map_open(*g);

        yajl_gen_string(*g, (const unsigned char *) STR_NAME, strlen(STR_NAME));
        if(mapp->name != NULL)
            yajl_gen_string(*g, (const unsigned char *) mapp->name, strlen (mapp->name));
        else
            yajl_gen_string(*g, (const unsigned char *) MAP_NO_NAME, strlen(MAP_NO_NAME));

        yajl_gen_string(*g, (const unsigned char *) STR_DESCRIPTION, strlen(STR_DESCRIPTION));
        if(mapp->description != NULL)
            yajl_gen_string(*g, (const unsigned char *) mapp->description, strlen (mapp->description));
        else
            yajl_gen_string(*g, (const unsigned char *) MAP_NO_DESCRIPTION, strlen(MAP_NO_DESCRIPTION));

        yajl_gen_string(*g, (const unsigned char *) STR_VERSION, strlen(STR_VERSION));
        if(mapp->version != NULL)
            yajl_gen_string(*g, (const unsigned char *) mapp->version, strlen(mapp->version));
        else
            yajl_gen_string(*g, (const unsigned char *) MAP_NO_VERSION, strlen(MAP_NO_VERSION));

        //Mapping_out
        hashCount = HASH_COUNT(mapp->map_elements);
        if (hashCount) {
            yajl_gen_string(*g, (const unsigned char *) "mapping_out", strlen("mapping_out"));
            yajl_gen_array_open(*g);
            for(currentMapOut = mapp->map_elements; currentMapOut != NULL; currentMapOut=currentMapOut->hh.next) {
                json_dump_mapping_out(g, currentMapOut);
            }
            yajl_gen_array_close(*g);
        }

    //    //Mapping_cat
    //    hashCount = HASH_COUNT(mapp->map_cat);
    //    if (hashCount) {
    //        yajl_gen_string(*g, (const unsigned char *) "mapping_cat", strlen("mapping_cat"));
    //        yajl_gen_array_open(*g);
    //        for(currentMapCat=mapp->map_cat; currentMapCat != NULL; currentMapCat=currentMapOut->hh.next) {
    //            json_dump_mapping_cat(g, currentMapCat);
    //        }
    //        yajl_gen_array_close(*g);
    //    }

        yajl_gen_map_close(*g);
    }
}

////////////////////////////////////////////////////////////////////////
// PRIVATE API
////////////////////////////////////////////////////////////////////////
/*
 * Function: load_definition
 * ----------------------------
 *   Load a agent definition in the standartised format JSON to initialize a definition structure from a string.
 *   The definition structure is dynamically allocated. You will have to use definition_freeDefinition function to deallocated it correctly.
 *
 *   json_str      : a string (json format)
 *
 *   returns: a pointer on a category structure or NULL if it has failed
 */

definition* parser_loadDefinition (const char* json_str) {
    
    definition *def = NULL;
    yajl_val node;
    
    json_tokenize(json_str, &node);
    def = json_parse_definition(node);
    
    yajl_tree_free(node);
    node = NULL;
    
    return def;
}

/*
 * Function: load_definition_from_path
 * -----------------------------------
 *   Load a agent definition in the standartised format JSON to initialize a definition structure from a local file path.
 *   The definition structure is dynamically allocated. You will have to use definition_freeDefinition function to deallocated it correctly.
 *
 *   file_path      : the file path
 *
 *   returns: a pointer on a category structure or NULL if it has failed
 */

definition * parser_loadDefinitionFromPath (const char* path) {
    
    char *json_str = NULL;
    definition *def = NULL;
    
    json_str = json_fetch(path);
    if (!json_str)
        return 0;
    
    def = parser_loadDefinition(json_str);
    
    free (json_str);
    json_str = NULL;
    
    return def;
}

/*
 * Function: parser_export_definition
 * ----------------------------
 *   Returns a agent's definition structure into a standartised format json string UTF8 to send it throught the BUS or save it in a file
 *
 *   def    : the agent's definition dump in string
 *
 *   returns: a definition json format string UTF8
 */

char* parser_export_definition (definition* def) {
    
    char* result = NULL;
    if (def != NULL){
        const unsigned char * json_str = NULL;
        size_t len;
        yajl_gen g;
        
        g = yajl_gen_alloc(NULL);
        yajl_gen_config(g, yajl_gen_beautify, 1);
        yajl_gen_config(g, yajl_gen_validate_utf8, 1);
        
        yajl_gen_map_open(g);
        yajl_gen_string(g, (const unsigned char *) STR_DEFINITION, strlen(STR_DEFINITION));
        
        if(def != NULL){
            json_dump_definition(&g, def);
        }
        yajl_gen_map_close(g);
        
        // try to get our dumping result
        if (yajl_gen_get_buf(g, &json_str, &len) == yajl_gen_status_ok){
            result = strdup((const char*) json_str);
        }
        
        yajl_gen_free(g);
    }
    return result;
}

/*
 * Function: parser_export_mapping
 * ----------------------------
 *   Returns a agent's mapping structure into a standartised format json string UTF8 to send it throught the BUS or save it in a file
 *
 *   mapp    : the agent's mapping dump in string
 *
 *   returns: a mapping json format string UTF8
 */

char* parser_export_mapping(mapping_t *mapp){
    char* result = NULL;
    if (mapp != NULL){
        const unsigned char * json_str = NULL;
        size_t len;
        yajl_gen g;
        
        g = yajl_gen_alloc(NULL);
        yajl_gen_config(g, yajl_gen_beautify, 1);
        yajl_gen_config(g, yajl_gen_validate_utf8, 1);
        
        yajl_gen_map_open(g);
        yajl_gen_string(g, (const unsigned char *) "mapping", strlen("mapping"));
        json_dump_mapping(&g, mapp);
        yajl_gen_map_close(g);
        
        // try to get our dumping result
        if (yajl_gen_get_buf(g, &json_str, &len) == yajl_gen_status_ok)
        {
            result = strdup((const char*) json_str);
        }
        
        yajl_gen_free(g);
    }
    return result;
}

/*
 * Function: load_map
 * ------------------
 *   Load a mapping in the standartised format JSON to initialize a mapping structure from a string.
 *   The mapping structure is dynamically allocated. You will have to use free_mapping function to deallocated it correctly.
 *
 *   json_str      : a string (json format)
 *
 *   returns : a pointer on a mapping structure or NULL if it has failed
 */

mapping_t* parser_LoadMap(const char* json_str){
    
    mapping_t *mapp = NULL;
    yajl_val node;
    
    json_tokenize(json_str, &node);
    mapp = json_parse_mapping (node);
    
    //Copy the mapp structure to the global variable map
    //copy_to_map_global(mapp);
    
    yajl_tree_free(node);
    node = NULL;
    
    return mapp;
}

/*
 * Function: load_map_from_path
 * ----------------------------
 *   Load a mapping in the standartised format JSON to initialize a mapping structure from a local file path.
 *   The mapping structure is dynamically allocated. You will have to use free_mapping function to deallocated it correctly.
 *
 *   file_path      : the file path
 *
 *   returns : a pointer on a mapping structure or NULL if it has failed
 */

mapping_t* parser_LoadMapFromPath (const char* path){
    
    char *json_str = NULL;
    mapping_t *mapp = NULL;
    
    json_str = json_fetch(path);
    if (!json_str)
        return NULL;
    
    mapp = parser_LoadMap(json_str);
    
    free (json_str);
    json_str = NULL;
    
    return mapp;
}


////////////////////////////////////////////////////////////////////////
// PUBLIC API
////////////////////////////////////////////////////////////////////////

/**
 * \fn int igs_loadDefinition (const char* json_str)
 * \ingroup loadSetGetDefFct
 * \brief load definition in variable 'igs_definition_loaded' & copy in 'igs_internal_definition"
 *      from a json string
 *
 * \param json_str String in json format. Can't be NULL.
 * \return The error. 1 is OK, 0 json string is NULL, -1 Definition file has not been loaded
 */
int igs_loadDefinition (const char* json_str){
    
    //Check if the json string is null
    if(json_str == NULL)
    {
        igs_debug("igs_loadDefinition : json string is null \n");
        return 0;
    }

    //Try to load definition
    definition *tmp = parser_loadDefinition(json_str);

    if(tmp == NULL)
    {
        igs_debug("igs_loadDefinition : json string caused an error and was ignored\n%s\n", json_str );
        return -1;
    }else{
        if (igs_internal_definition != NULL){
            definition_freeDefinition(igs_internal_definition);
            igs_internal_definition = NULL;
        }
        igs_internal_definition = tmp;
        //Check the name of agent from network layer
        char *name = igs_getAgentName();
        if(strcmp(name, AGENT_NAME_DEFAULT) == 0){
            //The name of the agent is default : we change it to definition name
            igs_setAgentName(igs_internal_definition->name);
        }//else
            //The agent name was assigned by the developer : we keep it untouched
        free(name);
        network_needToSendDefinitionUpdate = true;
    }

    return 1;
}

/**
 * \fn int igs_loadDefinitionFromPath (const char* file_path)
 * \ingroup loadSetGetDefFct
 * \brief load definition in variable 'igs_definition_loaded' & copy in 'igs_internal_definition"
 *      from a file path
 *
 * \param file_path The string which contains the json file path. Can't be NULL.
 * \return The error. 1 is OK, 0 json string is NULL, -1 Definition file has not been loaded
 */
int igs_loadDefinitionFromPath (const char* file_path){
    
    //Check if the json string is null
    if(file_path == NULL){
        igs_error("Json file path is NULL");
        return 0;
    }
    
    if (strlen(file_path) == 0){
        igs_debug("Json file path is empty");
        return 1;
    }

    //Try to load definition
    definition *tmp = parser_loadDefinitionFromPath(file_path);
    

    if(tmp == NULL)
    {
        igs_debug("igs_loadDefinitionFromPath : %s caused an error and was ignored\n", file_path);
        return -1;
    }else{
        strncpy(definitionPath, file_path, MAX_PATH - 1);
        if (igs_internal_definition != NULL){
            definition_freeDefinition(igs_internal_definition);
            igs_internal_definition = NULL;
        }
        igs_internal_definition = tmp;
        //Check the name of agent from network layer
        char *name = igs_getAgentName();
        if(strcmp(name, AGENT_NAME_DEFAULT) == 0){
            //The name of the agent is default : we change it to definition name
            igs_setAgentName(igs_internal_definition->name);
        }//else
            //The agent name was assigned by the developer : we keep it untouched
        free(name);
        network_needToSendDefinitionUpdate = true;
    }

    return 1;
}

