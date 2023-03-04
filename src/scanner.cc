// based on https://github.com/nickel-lang/tree-sitter-nickel/blob/main/src/scanner.cc

#include <cstring>
#include <tree_sitter/parser.h>
#include <cwctype>  // iswspace
#include <stdint.h> // uint8_t, int32_t
#include <string>
#include <vector>
#include <map>

#define DEBUG false
#define DEBUG_LOOPS false
#define DEADLOOP_MAX 30



namespace {

//using std::vector;



enum TokenType {
  RECIPEPREFIX,
  RECIPEPREFIX_ASSIGNMENT_OPERATOR,
  RECIPEPREFIX_ASSIGNMENT_VALUE,
  /*
  RECIPEPREFIX_VALUE,
  VARIABLE_ASSIGNMENT_NAME,
  VARIABLE_ASSIGNMENT_OPERATOR,
  VARIABLE_ASSIGNMENT_VALUE,
  */
};



struct Scanner {

  // debug

  int deadloop_counter;



  // state

  char recipe_prefix;

  //std::string variable_name;
  std::string variable_operator;
  std::string variable_value;
  //std::map<std::string, std::string> variables_map; // TODO sorted map



  Scanner() {
    // constructor
    recipe_prefix = '\t';
    deadloop_counter = 0;
  }



  inline bool serialize_string(
      char *buffer,
      uint8_t *pos,
      const std::string *src
    ) {

    if (*pos + src->size() + 1 > TREE_SITTER_SERIALIZATION_BUFFER_SIZE) {
      if (DEBUG) printf("scanner.cc: serialize_string: buffer exceeded: %lu > %i\n", (*pos + src->size() + 1), TREE_SITTER_SERIALIZATION_BUFFER_SIZE);
      return false;
    }
    memcpy(&buffer[*pos], src->c_str(), src->size() + 1);
    *pos += src->size() + 1;
    // debug
    /*
    if (DEBUG) printf("scanner.cc: serialize_string: buffer = ");
    for (uint8_t i = 0; i < *pos; i++) {
      char c = buffer[i];
      //if (('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || ('0' <= c && c <= '9')) {
      if (c == '\t') printf("\\t");
      else if (c == '\n') printf("\\n");
      else if (c == '\r') printf("\\r");
      else if (c == 0) printf("\\0");
      else printf("%c", c);
    }
    printf("\n");
    */
    return true;
  }



  unsigned serialize(char *buffer) {

    // dump state to byte array

    if (DEBUG) printf("scanner.cc: serialize\n");
    uint8_t pos = 0;
    buffer[0] = recipe_prefix;
    buffer[1] = 0;
    pos = 2;
    //if (!serialize_string(buffer, &pos, &variable_name)) return 0;
    if (!serialize_string(buffer, &pos, &variable_operator)) return 0;
    if (!serialize_string(buffer, &pos, &variable_value)) return 0;
    /*
    for (const auto &keyval : variables_map) {
      //printf("scanner.cc: serialize: key = '%s'\n", keyval.first.c_str());
      if (!serialize_string(buffer, &pos, &(keyval.first))) return 0;
      //printf("scanner.cc: serialize: val = '%s'\n", keyval.second.c_str());
      if (!serialize_string(buffer, &pos, &(keyval.second))) return 0;
    }
    */
    return pos;
  }



  inline bool deserialize_string(
      const char *buffer,
      uint8_t length,
      uint8_t *pos,
      std::string *dst
    ) {

    if (*pos > length) {
      return false;
    }
    *dst = "";
    // off by one error?
    for (const char *c = &buffer[*pos]; *c != 0; c++, (*pos)++) {
      //printf("scanner.cc: deserialize_string: c = dec %i = '%c'\n", (int) *c, *c);
      *dst += *c;
      if (*pos > length) {
        *dst = "";
        return false;
      }
    }
    (*pos)++;
    return true;
  }



  void deserialize(const char *buffer, uint8_t length) {

    // load state from byte array

    if (DEBUG) printf("scanner.cc: deserialize %i\n", length);

    uint8_t pos = 0;

    if (length < 2) {
      return;
    }
    recipe_prefix = buffer[0];
    pos += 2;

    if (DEBUG) {
      if (recipe_prefix == '\t') printf("scanner.cc: deserialize: recipe_prefix = '\\t'\n");
      else printf("scanner.cc: deserialize: recipe_prefix = '%c'\n", recipe_prefix);
    }

    //if (!deserialize_string(buffer, length, &pos, &variable_name)) return;
    //printf("scanner.cc: deserialize: variable_name = '%s'\n", variable_name.c_str());
    if (!deserialize_string(buffer, length, &pos, &variable_operator)) return;
    if (DEBUG) printf("scanner.cc: deserialize: variable_operator = '%s'\n", variable_operator.c_str());
    if (!deserialize_string(buffer, length, &pos, &variable_value)) return;
    if (DEBUG) printf("scanner.cc: deserialize: variable_value = '%s'\n", variable_value.c_str());

    std::string key;
    std::string val;
    deadloop_counter = 0;

    // FIXME only one keyval pair is serialized / deserialized

    /*
    while (true) {
      if (!deserialize_string(buffer, length, &pos, &key)) break;
      //printf("scanner.cc: deserialize: key = '%s'\n", key.c_str());
      if (key.size() == 0) {
        // key must not be empty
        break;
      }
      if (!deserialize_string(buffer, length, &pos, &val)) break;
      //printf("scanner.cc: deserialize: val = '%s'\n", val.c_str());
      variables_map[key] = val;
      if (DEBUG_LOOPS && ++deadloop_counter > DEADLOOP_MAX) abort();
    }

    for (const auto &keyval : variables_map) {
      if (DEBUG) printf("scanner.cc: deserialize: variables_map[%s] = '%s'\n", keyval.first.c_str(), keyval.second.c_str());
    }
    */
  }

  inline void skip(TSLexer *lexer) {
    lexer->advance(lexer, true);
  }

  inline void advance(TSLexer *lexer) {
    lexer->advance(lexer, false);
  }

  inline int32_t lookahead(TSLexer *lexer) {
    return lexer->lookahead;
  }



  bool scan_recipeprefix(TSLexer *lexer) {

    if (DEBUG) printf("scanner.cc: valid_symbols[RECIPEPREFIX]\n");
    if (lookahead(lexer) == recipe_prefix) {
      advance(lexer);
      lexer->result_symbol = RECIPEPREFIX;
      return true;
    }
    return false;
  }



  bool scan_recipeprefix_assignment_operator(TSLexer *lexer) {

    if (DEBUG) printf("scanner.cc: valid_symbols[RECIPEPREFIX_ASSIGNMENT_OPERATOR]\n");
    variable_operator = "";
    /*
    // skip whitespace
    while (iswspace(lookahead(lexer))) {
      skip(lexer);
    }
    */
    char next_char = lookahead(lexer);
    deadloop_counter = 0;
    while (!iswspace(next_char)) { // TODO better? this allows all non-space
      variable_operator += next_char;
      advance(lexer);
      next_char = lookahead(lexer);
      if (next_char == 0) {
        // eof
        return false;
      }
      if (DEBUG_LOOPS && ++deadloop_counter > DEADLOOP_MAX) abort();
    }
    if (variable_operator.size() > 0) {
      if (DEBUG) printf("scanner.cc:150: variable_operator = '%s'\n", variable_operator.c_str());
      lexer->result_symbol = RECIPEPREFIX_ASSIGNMENT_OPERATOR;
      return true;
    }
    return false;
  }



  /*
  bool scan_variable_assignment_name(TSLexer *lexer) {

    if (DEBUG) printf("scanner.cc:90: valid_symbols[VARIABLE_ASSIGNMENT_NAME]\n");

    variable_name = "";
    // skip whitespace
    //while (iswspace(lookahead(lexer))) {
    //  skip(lexer);
    //}
    // skip newlines
    char next_char = lookahead(lexer);
    deadloop_counter = 0;
    while (next_char == '\n') {
      skip(lexer);
      next_char = lookahead(lexer);
      if (DEBUG_LOOPS && ++deadloop_counter > DEADLOOP_MAX) abort();
    }

    next_char = lookahead(lexer);
    //printf("scanner.cc:105: next_char = dec %i = '%c'\n", (int) next_char, next_char);
    deadloop_counter = 0;
    while (!iswspace(next_char)) { // TODO better? this allows all non-space
      variable_name += next_char;
      advance(lexer);
      next_char = lookahead(lexer);
      if (next_char == 0) {
        // eof
        return false;
      }
      //printf("scanner.cc:111: next_char = dec %i = '%c'\n", (int) next_char, next_char);
      if (DEBUG_LOOPS && ++deadloop_counter > DEADLOOP_MAX) abort();
    }

    if (variable_name.size() == 0) {
      if (DEBUG) printf("scanner.cc:120: variable_name is empty\n");
      return false;
    }

    // mark end before lookahead
    lexer->mark_end(lexer);

    // FIXME lookahead to assignment_operator

    // FIXME dont parse rule headers:
    // scanner.cc:115: variable_name = 'all:'
    // assignments look like:
    // key := value
    // key += value

    // skip whitespace
    // FIXME handle escaped newlines = \ + \n
    next_char = lookahead(lexer);
    if (DEBUG) printf("scanner.cc:240: next_char = dec %i = '%c'\n", (int) next_char, next_char);
    while (iswspace(next_char)) {
      skip(lexer);
      next_char = lookahead(lexer);
      if (DEBUG) printf("scanner.cc:250: next_char = dec %i = '%c'\n", (int) next_char, next_char);
    }

    // next token must be operator: := or +=
    // TODO more operators?

    if (!(next_char == ':' || next_char == '+')) {
      if (DEBUG) printf("scanner.cc:250: no variable name: '%s'\n", variable_name.c_str());
      return false;
    }
    skip(lexer);
    next_char = lookahead(lexer);
    if (DEBUG) printf("scanner.cc:260: next_char = dec %i = '%c'\n", (int) next_char, next_char);

    if (!(next_char == '=')) {
      if (DEBUG) printf("scanner.cc:260: no variable name: '%s'\n", variable_name.c_str());
      return false;
    }

    if (DEBUG) printf("scanner.cc:265: variable_name = '%s'\n", variable_name.c_str());
    lexer->result_symbol = VARIABLE_ASSIGNMENT_NAME;
    return true;
  }
  */



  /*
  bool scan_variable_assignment_value(TSLexer *lexer) {

    if (DEBUG) printf("scanner.cc:160: valid_symbols[VARIABLE_ASSIGNMENT_VALUE]\n");
    variable_value = "";
    char next_char = lookahead(lexer);
    // skip whitespace
    while (iswspace(next_char) && next_char != '\n') {
      skip(lexer);
      next_char = lookahead(lexer);
    }
    deadloop_counter = 0;
    while (next_char != '\n') { // TODO better? this scans to end-of-line
      variable_value += next_char;
      advance(lexer);
      next_char = lookahead(lexer);
      if (DEBUG_LOOPS && ++deadloop_counter > DEADLOOP_MAX) abort();
    }
    // TODO parse value. expand variables etc
    if (DEBUG) printf("scanner.cc:175: variable_value = '%s'\n", variable_value.c_str());
    // update value
    if (
      //variable_operator == "=" || // has no effect in makefile?
      variable_operator == ":="
    ) {
      variables_map[variable_name] = variable_value;
    }
    else if (variable_operator == "+=") {
      if (variables_map.count(variable_name) == 1) {
        variables_map[variable_name] += variable_value;
      }
      else {
        variables_map[variable_name] = variable_value;
      }
    }
    else {
      if (DEBUG) printf("scanner.cc:190: FIXME handle variable_operator = '%s'\n", variable_operator.c_str());
      variables_map[variable_name] = variable_value;
    }
    if (DEBUG) printf("scanner.cc:190: variables_map[%s] = '%s'\n", variable_name.c_str(), variable_value.c_str());

    variable_name = "";
    variable_operator = "";
    variable_value = "";

    lexer->result_symbol = VARIABLE_ASSIGNMENT_VALUE;
    return true;
  }
  */



  bool scan_recipeprefix_assignment_value(TSLexer *lexer) {

    // this is a more specific variant of scan_variable_assignment_value

    if (DEBUG) printf("scanner.cc: valid_symbols[RECIPEPREFIX_ASSIGNMENT_VALUE]\n");

    // value of .RECIPEPREFIX variable
    // only the first char is used as indent
    // https://www.gnu.org/software/make/manual/html_node/Recipe-Syntax.html
    // popular values for RECIPEPREFIX
    // https://github.com/search?q=RECIPEPREFIX+filename%3Amakefile

    char next_char = lookahead(lexer);
    //printf("scanner.cc: next_char = dec %i = '%c'\n", (int) next_char, next_char);

    // skip whitespace
    deadloop_counter = 0;
    while (iswspace(next_char) && next_char != '\n') {
      if (DEBUG) printf("scanner.cc: skip whitespace\n");
      skip(lexer);
      next_char = lookahead(lexer);
      //printf("scanner.cc: next_char = dec %i = '%c'\n", (int) next_char, next_char);
      if (DEBUG_LOOPS && ++deadloop_counter > DEADLOOP_MAX) abort();
    }

    // scan to end of line
    variable_value = "";
    next_char = lookahead(lexer);
    deadloop_counter = 0;
    while (next_char != '\n') {
      variable_value += next_char;
      advance(lexer);
      next_char = lookahead(lexer);
      if (DEBUG_LOOPS && ++deadloop_counter > DEADLOOP_MAX) abort();
    }

    // set recipe_prefix
    {
      if (variable_value.size() == 0) {
        // FIXME variable_operator is not valid here
        // TODO parse only the RECIPEPREFIX assignment in scanner.cc
        // and ignore all other assignments
        if (variable_operator == "+=") {
          // .RECIPEPREFIX +=
          // note: there is nothing after the "+="
          // backward-compatible with make 4.2 and before
          // this does not work with make 4.3 and after (2020-01-19)
          // append space to empty value -> space
          recipe_prefix = ' ';
        }
        else {
          // reset to default value
          recipe_prefix = '\t';
        }
      }
      // else: variable_value[0] == '$'
      else if (
        variable_value.size() >= 2 && (
          variable_value[1] == '(' ||
          variable_value[1] == '{'
        )
      ) {
        // variable_value is 2 or longer and starts with $( or ${
        // some ways to indent with spaces
        // see also https://stackoverflow.com/questions/2131213/can-you-make-valid-makefiles-without-tab-characters
        // these do not require keeping track of variables like $(space)
        // $() and ${} are empty and can be used as delimiters
        // we care only about the first char after $() or ${}
        // so the full value can be "$() anything"
        // can even be a parse error like "$() $(" -> error: unterminated variable reference
        // this code could be faster, but its rarely used, so we dont care
        if (
          // safe
          // this is rarely used in practice
          // .RECIPEPREFIX := $() $()
          // .RECIPEPREFIX := $() #
          variable_value.substr(0, 4) == "$() " ||
          variable_value.substr(0, 4) == "${} " ||

          // risky. this assumes: .RECIPEPREFIX := $()
          // this is often used in practice
          // .RECIPEPREFIX := $(.RECIPEPREFIX) $(.RECIPEPREFIX)
          // .RECIPEPREFIX := $(.RECIPEPREFIX) #
          variable_value.substr(0, 17) == "$(.RECIPEPREFIX) " ||
          variable_value.substr(0, 17) == "${.RECIPEPREFIX} " ||

          // risky. this assumes: space := $() $()
          // this is rarely used in practice
          // based on https://www.gnu.org/software/make/manual/html_node/Syntax-of-Functions.html#Special-Characters
          // space := $() $()
          // .RECIPEPREFIX := $(space)
          variable_value == "$(space)" ||
          variable_value == "${space}"
        ) {
          recipe_prefix = ' ';
        }
        else {
          // expression is too complex.
          // ideally we would evaluate the expression
          // but this would require keeping track of variables.
          // backtracking is not an option:
          // http://tree-sitter.github.io/tree-sitter/creating-parsers#external-scanners
          // > you cannot backtrack
          // example:
          // space_char := $() $()
          // .RECIPEPREFIX := $(space_char)
          // cheap fix: fallback to default value
          recipe_prefix = '\t';
          if (DEBUG) printf("scanner.cc: FIXME evaluate variable_value = '%s'\n", variable_value.c_str());
        }
      }
      else {
        // variable_value is 1 or longer and does NOT start with $
        // value is a literal string
        // no need to eval, just take the first char
        // examples:
        // .RECIPEPREFIX := > # indent with '>'. often used in practice
        // .RECIPEPREFIX := asdf # indent with 'a'
        // .RECIPEPREFIX := \ # indent with '\\'. everything after the \ is ignored
        recipe_prefix = variable_value[0];
      }
    }

    if (recipe_prefix == '\t') {
      if (DEBUG) printf("scanner.cc: recipe_prefix = '\\t'\n");
    }
    else {
      if (DEBUG) printf("scanner.cc: recipe_prefix = '%c'\n", recipe_prefix);
    }

    // reset temporary variables
    variable_operator = "";
    variable_value = "";

    // no. we already are at end of line
    /*
    next_char = lookahead(lexer);
    //printf("scanner.cc: next_char = dec %i = '%c'\n", (int) next_char, next_char);
    deadloop_counter = 0;
    while (next_char != '\n') {
      advance(lexer);
      next_char = lookahead(lexer);
      //printf("scanner.cc: next_char = dec %i = '%c'\n", (int) next_char, next_char);
      if (DEBUG_LOOPS && ++deadloop_counter > DEADLOOP_MAX) abort();
    }
    */

    lexer->result_symbol = RECIPEPREFIX_ASSIGNMENT_VALUE;
    return true;
  }



  bool scan(TSLexer *lexer, const bool *valid_symbols) {

    // router for all scan_* functions

    // During error recovery we don't run the external scanner. This produces
    // less accurate results, but avoids a large deal of complexity in this
    // scanner.
    if (
      // TODO keep in sync with enum TokenType
      valid_symbols[RECIPEPREFIX] &&
      valid_symbols[RECIPEPREFIX_ASSIGNMENT_OPERATOR] &&
      valid_symbols[RECIPEPREFIX_ASSIGNMENT_VALUE]
    ) {
      return false;
    }

    // Skip over all whitespace
    /*
    while (iswspace(lookahead(lexer))) {
      skip(lexer);
    }
    */

    // often used (every line)
    if (valid_symbols[RECIPEPREFIX]) {
      return scan_recipeprefix(lexer);
    }

    // rarely used
    if (valid_symbols[RECIPEPREFIX_ASSIGNMENT_OPERATOR]) {
      return scan_recipeprefix_assignment_operator(lexer);
    }

    if (valid_symbols[RECIPEPREFIX_ASSIGNMENT_VALUE]) {
      return scan_recipeprefix_assignment_value(lexer);
    }

    /*
    if (valid_symbols[VARIABLE_ASSIGNMENT_NAME]) {
      return scan_variable_assignment_name(lexer);
    }

    if (valid_symbols[VARIABLE_ASSIGNMENT_OPERATOR]) {
      return scan_variable_assignment_operator(lexer);
    }

    if (valid_symbols[VARIABLE_ASSIGNMENT_VALUE]) {
      return scan_variable_assignment_value(lexer);
    }
    */

    /*
        word: $ => token(repeat1(choice(
            new RegExp ('['+CHARSET+']'),
            new RegExp ('\\\\['+ESCAPE_SET+']'),
            new RegExp ('\\\\[0-9]{3}'),
        ))),
    */

    if (DEBUG) printf("scanner.cc: valid_symbols is empty\n");

    return false;
  }
}; // struct Scanner

} // namespace



extern "C" {

void *tree_sitter_make_external_scanner_create() {
  return new Scanner();
}

void tree_sitter_make_external_scanner_destroy(void *payload) {
  Scanner *scanner = static_cast<Scanner *>(payload);
  delete scanner;
}

bool tree_sitter_make_external_scanner_scan(
    void *payload,
    TSLexer *lexer,
    const bool *valid_symbols
  ) {
  Scanner *scanner = static_cast<Scanner *>(payload);
  return scanner->scan(lexer, valid_symbols);
}

/**
 * @param Contains the scanner
 * @param Will hold the serialized state of the scanner
 */
unsigned tree_sitter_make_external_scanner_serialize(
    void *payload,
    char *buffer
  ) {
  Scanner *scanner = static_cast<Scanner *>(payload);
  return scanner->serialize(buffer);
}

/**
 * @param Contains the scanner
 * @param The serialised state of the scanner
 * @param Indicates the length of the buffer
 */
void tree_sitter_make_external_scanner_deserialize(
    void *payload,
    const char *buffer,
    unsigned length
  ) {
  Scanner *scanner = static_cast<Scanner *>(payload);
  uint8_t length_uint8 = (uint8_t)length;
  scanner->deserialize(buffer, length_uint8);
}

} // extern "C"
