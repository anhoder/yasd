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

#pragma once

#include <string>
#include <vector>
#include <sstream>
#include <iostream>
#include <utility>

#include "main/php.h"

namespace yasd { namespace util {
namespace variable {
  HashTable *get_defined_vars();
  zval *find_variable(std::string var_name);
  zval *find_variable(zend_array *symbol_table, zend_ulong index);
  zval *find_variable(zend_array *symbol_table, std::string var_name);
  void print_var(std::string fullname);
  bool is_equal(zval *op1, zval *op2);
  bool is_smaller(zval *op1, zval *op2);
  bool is_greater(zval *op1, zval *op2);
}

namespace execution {
  const char *get_filename();
  const char *get_function_name(zend_function *func = nullptr);
  int get_file_lineno();
  const char *get_prev_filename();
  const char *get_prev_function_name();
  int get_prev_file_lineno();
  bool eval_string(char *str, zval *retval_ptr, char *string_name);
}

namespace string {
  std::string stripslashes(std::string str);
  std::string stripcslashes(std::string str);
  std::string addslashes(std::string str);
  bool is_substring(std::string sub_str, std::string target_str);
  bool is_integer(const std::string &s);
}

namespace time {
  long microtime(void);
}

namespace option {
  std::string get_value(const std::vector<std::string> &options, std::string option);
}
    
  void print_property(std::string obj_name, std::string property_name);

  void printf_info(int color, const char *format, ...);

  template <typename... Args>
  void printfln_info(int color, const char *format, Args... args) {
      printf_info(color, format, args...);
      std::cout << std::endl;
  }

  zend_array *get_properties(zval *zobj);

  // get the property name of a common property, including public, protected, private
  std::string get_property_name(zend_string *property_name);

  std::string wrap_property_name(zend_string *origin_cur_class_name, zend_string *origin_property_name);
  std::string unwrap_property_name(zend_string *origin_property_name);

  zval *fetch_zval_by_fullname(std::string fullname);
} // namespace util
}  // namespace yasd
