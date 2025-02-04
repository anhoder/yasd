/*
  +----------------------------------------------------------------------+
  | Yasd                                                                 |
  +----------------------------------------------------------------------+
  | This source file is subject to version 2.0 of the Apache license,    |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.apache.org/licenses/LICENSE-2.0.html                      |
  | If you did not receive a copy of the Apache2.0 license and are unable|
  | to obtain it through the world-wide-web, please send a note to       |
  | license@swoole.com so we can mail you a copy immediately.            |
  +----------------------------------------------------------------------+
  | Author: codinghuang  <codinghuang@qq.com>                            |
  +----------------------------------------------------------------------+
*/

#include <iostream>

#include "include/util.h"
#include "include/global.h"

#include "./php_yasd_cxx.h"

YASD_EXTERN_C_BEGIN
#include "ext/standard/php_var.h"
#include "ext/standard/php_string.h"
YASD_EXTERN_C_END

namespace yasd { namespace util {
namespace variable {

HashTable *get_defined_vars() {
    HashTable *symbol_table;

    symbol_table = zend_rebuild_symbol_table();

    return symbol_table;
}

zval *find_variable(std::string var_name) {
    HashTable *defined_vars;

    defined_vars = get_defined_vars();

    return find_variable(defined_vars, var_name);
}

zval *find_variable(zend_array *symbol_table, zend_ulong index) {
    zval *var;

    var = zend_hash_index_find(symbol_table, index);

    // not define variable
    if (!var) {
        return nullptr;
    }

    while (Z_TYPE_P(var) == IS_INDIRECT) {
        var = Z_INDIRECT_P(var);
    }

    return var;
}

zval *find_variable(zend_array *symbol_table, std::string var_name) {
    zval *var;

    if (var_name == "this") {
        return &EG(current_execute_data)->This;
    }

    var = zend_hash_str_find(symbol_table, var_name.c_str(), var_name.length());

    // not define variable
    if (!var) {
        return nullptr;
    }

    while (Z_TYPE_P(var) == IS_INDIRECT) {
        var = Z_INDIRECT_P(var);
    }

    return var;
}

// need to drop the $ for fullname
void print_var(std::string fullname) {
    zval *var;

    var = yasd::util::fetch_zval_by_fullname(fullname);

    if (!var) {
        yasd::util::printfln_info(yasd::Color::YASD_ECHO_GREEN, "not found variable $%s", fullname.c_str());
    } else {
        php_var_dump(var, 1);
    }
}

bool is_equal(zval *op1, zval *op2) {
    zval result;

    is_equal_function(&result, op1, op2);
    return Z_LVAL(result) == 0;
}

bool is_smaller(zval *op1, zval *op2) {
    zval result;
    is_smaller_function(&result, op1, op2);
    return zend_is_true(&result) == 1;
}

bool is_greater(zval *op1, zval *op2) {
    zval result;
    is_smaller_or_equal_function(&result, op1, op2);
    return zend_is_true(&result) == 0;
}
} // namespace variable

namespace execution {

const char *get_filename() {
    zend_string *filename;

    filename = zend_get_executed_filename_ex();

    return ZSTR_VAL(filename);
}

const char *get_function_name(zend_function *func) {
    if (func == nullptr) {
        zend_execute_data *ptr = EG(current_execute_data);
        while (ptr && (!ptr->func || !ZEND_USER_CODE(ptr->func->type))) {
            ptr = ptr->prev_execute_data;
        }

        if (ptr && ptr->func && ptr->func->op_array.function_name) {
            return ZSTR_VAL(ptr->func->op_array.function_name);
        }
    } else {
        if (func->common.function_name) {
            return func->common.function_name->val;
        }
    }
    
    return "main";
}

int get_file_lineno() {
    if (!EG(current_execute_data)) {
        return 0;
    }
    return EG(current_execute_data)->opline->lineno;
}

const char *get_prev_filename() {
    zend_execute_data *ptr = EG(current_execute_data)->prev_execute_data;

    while (ptr && (!ptr->func || !ZEND_USER_CODE(ptr->func->type))) {
        ptr = ptr->prev_execute_data;
    }

    if (ptr && ptr->func) {
        return ptr->func->op_array.filename->val;
    }

    return "unknow file";
}

const char *get_prev_function_name() {
    zend_execute_data *ptr = EG(current_execute_data)->prev_execute_data;

    while (ptr && (!ptr->func || !ZEND_USER_CODE(ptr->func->type))) {
        ptr = ptr->prev_execute_data;
    }

    if (ptr && ptr->func && ptr->func->op_array.function_name) {
        return ZSTR_VAL(ptr->func->op_array.function_name);
    }
    return "main";
}

int get_prev_file_lineno() {
    zend_execute_data *ptr = EG(current_execute_data)->prev_execute_data;

    while (ptr && (!ptr->func || !ZEND_USER_CODE(ptr->func->type))) {
        ptr = ptr->prev_execute_data;
    }

    if (ptr && ptr->opline) {
        return ptr->opline->lineno;
    }

    return 0;
}

bool eval_string(char *str, zval *retval_ptr, char *string_name) {
    int origin_error_reporting = EG(error_reporting);

    // we need to turn off the warning if the variable is UNDEF
    EG(error_reporting) = 0;

    int ret;
    ret = zend_eval_string(str, retval_ptr, const_cast<char *>("yasd://debug-eval"));
    if (ret == FAILURE) {
        return false;
    }

    EG(error_reporting) = origin_error_reporting;

    return true;
}
} // execution

namespace string {

std::string stripslashes(std::string str) {
    zend_string *tmp_zstr = zend_string_init(str.c_str(), str.length(), 0);
    php_stripslashes(tmp_zstr);
    str = std::string(ZSTR_VAL(tmp_zstr), ZSTR_LEN(tmp_zstr));
    zend_string_release(tmp_zstr);
    return str;
}

std::string stripcslashes(std::string str) {
    zend_string *tmp_zstr = zend_string_init(str.c_str(), str.length(), 0);
    php_stripcslashes(tmp_zstr);
    str = std::string(ZSTR_VAL(tmp_zstr), ZSTR_LEN(tmp_zstr));
    zend_string_release(tmp_zstr);
    return str;
}

std::string addslashes(std::string str) {
    zend_string *tmp_zstr = zend_string_init(str.c_str(), str.length(), 0);

# if PHP_VERSION_ID >= 70300
    tmp_zstr = php_addslashes(tmp_zstr);
# else
    tmp_zstr = php_addslashes(tmp_zstr, 0);
# endif
    str = std::string(ZSTR_VAL(tmp_zstr), ZSTR_LEN(tmp_zstr));
    zend_string_release(tmp_zstr);
    return str;
}

bool is_substring(std::string sub_str, std::string target_str) {
    for (size_t i = 0; i < sub_str.length(); i++) {
        if (sub_str[i] != target_str[i]) {
            return false;
        }
    }

    return true;
}

bool is_integer(const std::string &s) {
    if (s.empty() || ((!isdigit(s[0])) && (s[0] != '-') && (s[0] != '+'))) return false;

    char *p;
    strtol(s.c_str(), &p, 10);

    return (*p == 0);
}
} // namespace string

namespace time {

long microtime() {
    struct timeval t;
    gettimeofday(&t, nullptr);
    return t.tv_sec * 1000 + t.tv_usec / 1000;
}
} // namespace time

namespace option {

std::string get_value(const std::vector<std::string> &options, std::string option) {
    auto iter = options.begin();

    for (; iter != options.end(); iter++) {
        if (option == *iter) {
            break;
        }
    }

    return *(++iter);
}
}


void print_property(std::string obj_name, std::string property_name) {
    zval *obj;
    zval *property;
    zend_execute_data *execute_data = EG(current_execute_data);

    if (obj_name == "this") {
        property =
            yasd_zend_read_property(Z_OBJCE_P(ZEND_THIS), ZEND_THIS, property_name.c_str(), property_name.length(), 1);
        php_var_dump(property, 1);
    } else {
        obj = variable::find_variable(obj_name);
        if (!obj) {
            yasd::util::printfln_info(yasd::Color::YASD_ECHO_RED, "undefined variable %s", obj_name.c_str());
            return;
        }
        property = yasd_zend_read_property(Z_OBJCE_P(obj), obj, property_name.c_str(), property_name.length(), 1);
        if (!property) {
            yasd::util::printfln_info(yasd::Color::YASD_ECHO_GREEN,
                                      "undefined property %s::$%s",
                                      ZSTR_VAL(Z_OBJCE_P(obj)->name),
                                      property_name.c_str());
            return;
        }
        php_var_dump(property, 1);
    }

    return;
}

void printf_info(int color, const char *format, ...) {
    va_list args;
    va_start(args, format);
    vsnprintf(yasd_info_buf, sizeof(yasd_info_buf), format, args);
    va_end(args);

    switch (color) {
    case yasd::Color::YASD_ECHO_RED:
        std::cout << "\e[31m" << yasd_info_buf << "\e[0m";
        break;
    case yasd::Color::YASD_ECHO_GREEN:
        std::cout << "\e[32m" << yasd_info_buf << "\e[0m";
        break;
    case yasd::Color::YASD_ECHO_YELLOW:
        std::cout << "\e[33m" << yasd_info_buf << "\e[0m";
        break;
    case yasd::Color::YASD_ECHO_BLUE:
        std::cout << "\e[34m" << yasd_info_buf << "\e[0m";
        break;
    case yasd::Color::YASD_ECHO_MAGENTA:
        std::cout << "\e[35m" << yasd_info_buf << "\e[0m";
        break;
    case yasd::Color::YASD_ECHO_CYAN:
        std::cout << "\e[36m" << yasd_info_buf << "\e[0m";
        break;
    case yasd::Color::YASD_ECHO_WHITE:
        std::cout << "\e[37m" << yasd_info_buf << "\e[0m";
        break;
    default:
        break;
    }
}

zend_array *get_properties(zval *zobj) {
#if PHP_VERSION_ID >= 70400
    return zend_get_properties_for(zobj, ZEND_PROP_PURPOSE_VAR_EXPORT);
#else
    if (Z_OBJ_HANDLER_P(zobj, get_properties)) {
        return Z_OBJPROP_P(zobj);
    }
#endif
    return nullptr;
}

std::string wrap_property_name(zend_string *origin_cur_class_name, zend_string *origin_property_name) {
    const char *_class_name, *_property_name;
    size_t _property_name_len;

    zend_unmangle_property_name_ex(origin_property_name, &_class_name, &_property_name, &_property_name_len);

    std::string property_name = std::string(_property_name, _property_name_len);
    std::string cur_class_name = std::string(origin_cur_class_name->val, origin_cur_class_name->len);
    std::string class_name;
    if (_class_name) {
        class_name = std::string(_class_name, strlen(_class_name));
    }

    if (!_class_name || class_name == "*") {
        if (property_name.substr(0, 3) == std::string("\0*\0", 3)) {
            property_name = property_name.substr(3, property_name.length() - 3);
        }
        if (class_name == "*") {
            property_name = "*" + property_name;
        }
        return property_name;
    }

    return class_name + "*" + property_name;
}

std::string unwrap_property_name(std::string origin_property_name) {
    auto pos = origin_property_name.find('*');
    printf("pos: %ld\n", pos);
    if (pos == std::string::npos || pos == origin_property_name.length()) {
        return origin_property_name;
    }

    auto class_name = origin_property_name.substr(0, pos);
    auto property_name = origin_property_name.substr(pos+1);

    printf("class: %s, property: %s\n", class_name.c_str(), property_name.c_str());

    if (class_name.empty()) {
        return std::string("\0*\0", 3) + property_name;
    }
    auto property = zend_mangle_property_name(class_name.c_str(), class_name.length(), property_name.c_str(), property_name.length(), 0);
    auto res = std::string(property->val, property->len);
    zend_string_release(property);

    return res;
}

std::string get_property_name(zend_string *property_name) {
    const char *class_name, *_property_name;
    size_t _property_name_len;

    zend_unmangle_property_name_ex(property_name, &class_name, &_property_name, &_property_name_len);

    return std::string(_property_name, _property_name_len);
}

// TODO(codinghuang): maybe we can use re2c and yacc later
zval *fetch_zval_by_fullname(std::string fullname) {
    int state = 0;
    char quotechar = 0;
    const char *ptr = fullname.c_str();
    const char *end = fullname.c_str() + fullname.length() - 1;

    // aa->bb->cc
    // aa[bb][cc]
    // k[0]

    struct NextZvalInfo {
        enum NextZvalType {
            ARRAY_INDEX_ASSOC,
            ARRAY_INDEX_NUM,
        };

        std::string name;
        NextZvalType type;
        const char *keyword;
        const char *keyword_end = nullptr;
        zend_array *symbol_table;
        zval *retval_ptr = nullptr;
    };

    NextZvalInfo next_zval_info;
    next_zval_info.symbol_table = zend_rebuild_symbol_table();
    next_zval_info.keyword = ptr;

    auto fetch_next_zval = [](NextZvalInfo *next_zval_info) {
        std::string name(next_zval_info->keyword, next_zval_info->keyword_end - next_zval_info->keyword);

        if (!next_zval_info->retval_ptr && name == "GLOBALS") {
            next_zval_info->symbol_table = &EG(symbol_table);
            next_zval_info->retval_ptr = &global->globals;
            return;
        } else if (next_zval_info->retval_ptr) {
            if (Z_TYPE_P(next_zval_info->retval_ptr) == IS_OBJECT) {
                next_zval_info->symbol_table = get_properties(next_zval_info->retval_ptr);
                convert_to_array(next_zval_info->retval_ptr);
                php_var_dump(next_zval_info->retval_ptr, 0);
            } else {
                next_zval_info->symbol_table = Z_ARRVAL_P(next_zval_info->retval_ptr);
            }
        }

        if (!next_zval_info->retval_ptr) {
            next_zval_info->retval_ptr = variable::find_variable(next_zval_info->symbol_table, name);
        } else if (Z_TYPE_P(next_zval_info->retval_ptr) == IS_ARRAY) {
            if (next_zval_info->type == NextZvalInfo::NextZvalType::ARRAY_INDEX_NUM) {
                next_zval_info->retval_ptr =
                    variable::find_variable(next_zval_info->symbol_table, strtoull(name.c_str(), NULL, 10));
            } else {
                // name = yasd::util::string::stripslashes(name);
                name = unwrap_property_name(name);
                next_zval_info->retval_ptr = variable::find_variable(next_zval_info->symbol_table, name);
            }
        } else {
            next_zval_info->retval_ptr = yasd_zend_read_property(
                Z_OBJCE_P(next_zval_info->retval_ptr), next_zval_info->retval_ptr, name.c_str(), name.length(), 1);
        }
        next_zval_info->keyword = nullptr;
    };

    do {
        switch (state) {
        case 0:
            next_zval_info.keyword = ptr;
            state = 1;
        case 1:
            if (*ptr == '[') {
                next_zval_info.keyword_end = ptr;
                if (next_zval_info.keyword) {
                    fetch_next_zval(&next_zval_info);
                }
                state = 3;
            } else if (*ptr == '-') {
                next_zval_info.keyword_end = ptr;
                if (next_zval_info.keyword) {
                    fetch_next_zval(&next_zval_info);
                }
                state = 2;
            }
            break;
        case 2:
            assert(*ptr == '>');
            next_zval_info.keyword = ptr + 1;
            state = 1;
            break;
        case 3:
            if ((*ptr == '\'' || *ptr == '"')) {
                state = 4;
                quotechar = *ptr;
                next_zval_info.keyword = ptr + 1;
                next_zval_info.type = NextZvalInfo::NextZvalType::ARRAY_INDEX_ASSOC;
            } else if (*ptr >= '0' && *ptr <= '9') {
                state = 6;
                next_zval_info.keyword = ptr;
                next_zval_info.type = NextZvalInfo::NextZvalType::ARRAY_INDEX_NUM;
            }
            break;
        case 4:
            if (*ptr == '\\') {  // for example classname key array (Bar\\Foo::class)
                state = 10;
            } else if (*ptr == quotechar) {
                quotechar = 0;
                state = 5;
                next_zval_info.keyword_end = ptr;
                fetch_next_zval(&next_zval_info);
            }
            break;
        case 5:
            if (*ptr == ']') {
                state = 1;
            }
            break;
        case 6:
            if (*ptr == ']') {
                next_zval_info.keyword_end = ptr;
                if (next_zval_info.keyword) {
                    fetch_next_zval(&next_zval_info);
                }
                state = 1;
            }
            break;
        case 10: /* escaped character */
            state = 4;
            break;
        default:
            break;
        }
        ptr++;
    } while (ptr <= end);

    if (next_zval_info.keyword) {
        next_zval_info.keyword_end = ptr;

        fetch_next_zval(&next_zval_info);
    }

    return next_zval_info.retval_ptr;
}

} // namespace util

}  // namespace yasd
