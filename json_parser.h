// json_parser.h - Parser JSON ligero para mcp_client
#ifndef JSON_PARSER_H
#define JSON_PARSER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef enum {
    JSON_STRING,
    JSON_NUMBER,
    JSON_BOOL,
    JSON_NULL,
    JSON_OBJECT,
    JSON_ARRAY
} JsonType;

typedef struct JsonValue {
    JsonType type;
    union {
        char* string;
        double number;
        int boolean;
        struct JsonObject* object;
        struct JsonArray* array;
    } value;
} JsonValue;

typedef struct JsonPair {
    char* key;
    JsonValue* value;
    struct JsonPair* next;
} JsonPair;

typedef struct JsonObject {
    JsonPair* pairs;
} JsonObject;

typedef struct JsonArray {
    JsonValue** items;
    size_t count;
    size_t capacity;
} JsonArray;

// Parser functions
static char* json_skip_whitespace(char* str) {
    while (*str && isspace(*str)) str++;
    return str;
}

static char* json_parse_string(char* str, char** out) {
    if (*str != '"') return NULL;
    str++; // Skip opening quote
    
    char* start = str;
    size_t len = 0;
    
    while (*str && *str != '"') {
        if (*str == '\\' && *(str + 1)) {
            str += 2;
            len += 2;
        } else {
            str++;
            len++;
        }
    }
    
    if (*str != '"') return NULL;
    
    *out = malloc(len + 1);
    if (!*out) return NULL;
    
    char* dst = *out;
    char* src = start;
    
    while (src < str) {
        if (*src == '\\' && *(src + 1)) {
            src++;
            switch (*src) {
                case 'n': *dst++ = '\n'; break;
                case 't': *dst++ = '\t'; break;
                case 'r': *dst++ = '\r'; break;
                case '"': *dst++ = '"'; break;
                case '\\': *dst++ = '\\'; break;
                default: *dst++ = *src; break;
            }
            src++;
        } else {
            *dst++ = *src++;
        }
    }
    *dst = '\0';
    
    return str + 1; // Skip closing quote
}

static JsonValue* json_parse_value(char** str);

static JsonValue* json_parse_object(char** str) {
    if (**str != '{') return NULL;
    (*str)++; // Skip '{'
    
    JsonObject* obj = calloc(1, sizeof(JsonObject));
    JsonValue* result = calloc(1, sizeof(JsonValue));
    result->type = JSON_OBJECT;
    result->value.object = obj;
    
    *str = json_skip_whitespace(*str);
    
    if (**str == '}') {
        (*str)++;
        return result;
    }
    
    JsonPair** current = &obj->pairs;
    
    while (**str) {
        *str = json_skip_whitespace(*str);
        
        // Parse key
        char* key = NULL;
        *str = json_parse_string(*str, &key);
        if (!*str || !key) goto error;
        
        *str = json_skip_whitespace(*str);
        if (**str != ':') goto error;
        (*str)++; // Skip ':'
        
        *str = json_skip_whitespace(*str);
        
        // Parse value
        JsonValue* value = json_parse_value(str);
        if (!value) goto error;
        
        // Create pair
        JsonPair* pair = calloc(1, sizeof(JsonPair));
        pair->key = key;
        pair->value = value;
        
        *current = pair;
        current = &pair->next;
        
        *str = json_skip_whitespace(*str);
        
        if (**str == '}') {
            (*str)++;
            return result;
        }
        
        if (**str != ',') goto error;
        (*str)++; // Skip ','
    }
    
error:
    // TODO: Implementar liberación de memoria en caso de error
    return NULL;
}

static JsonValue* json_parse_value(char** str) {
    *str = json_skip_whitespace(*str);
    
    if (**str == '"') {
        char* string = NULL;
        *str = json_parse_string(*str, &string);
        if (!*str) return NULL;
        
        JsonValue* val = calloc(1, sizeof(JsonValue));
        val->type = JSON_STRING;
        val->value.string = string;
        return val;
    }
    
    if (**str == '{') {
        return json_parse_object(str);
    }
    
    if (strncmp(*str, "true", 4) == 0) {
        *str += 4;
        JsonValue* val = calloc(1, sizeof(JsonValue));
        val->type = JSON_BOOL;
        val->value.boolean = 1;
        return val;
    }
    
    if (strncmp(*str, "false", 5) == 0) {
        *str += 5;
        JsonValue* val = calloc(1, sizeof(JsonValue));
        val->type = JSON_BOOL;
        val->value.boolean = 0;
        return val;
    }
    
    if (strncmp(*str, "null", 4) == 0) {
        *str += 4;
        JsonValue* val = calloc(1, sizeof(JsonValue));
        val->type = JSON_NULL;
        return val;
    }
    
    // TODO: Implementar parseo de números y arrays
    
    return NULL;
}

// Helper functions
static JsonValue* json_object_get(JsonObject* obj, const char* key) {
    if (!obj || !key) return NULL;
    
    JsonPair* pair = obj->pairs;
    while (pair) {
        if (strcmp(pair->key, key) == 0) {
            return pair->value;
        }
        pair = pair->next;
    }
    return NULL;
}

static char* json_get_string(JsonObject* obj, const char* key) {
    JsonValue* val = json_object_get(obj, key);
    if (val && val->type == JSON_STRING) {
        return val->value.string;
    }
    return NULL;
}

static int json_get_bool(JsonObject* obj, const char* key, int default_value) {
    JsonValue* val = json_object_get(obj, key);
    if (val && val->type == JSON_BOOL) {
        return val->value.boolean;
    }
    return default_value;
}

// Cleanup
static void json_free_value(JsonValue* val) {
    if (!val) return;
    
    switch (val->type) {
        case JSON_STRING:
            free(val->value.string);
            break;
        case JSON_OBJECT:
            {
                JsonPair* pair = val->value.object->pairs;
                while (pair) {
                    JsonPair* next = pair->next;
                    free(pair->key);
                    json_free_value(pair->value);
                    free(pair);
                    pair = next;
                }
                free(val->value.object);
            }
            break;
        case JSON_ARRAY:
            // TODO: Implementar liberación de arrays
            break;
        default:
            break;
    }
    free(val);
}

#endif // JSON_PARSER_H
