/* ts_defs.l — Shared TypeScript definitions (source of truth)
 *
 * This file is NOT compiled directly. It contains flex definitions
 * that are kept in sync between lexer_ts.l and lexer_tsx.l.
 * When modifying keywords, update this file AND both .l files.
 *
 * JavaScript base keywords (shared by JS/TS/TSX)
 */
DIGIT          [0-9]
NUM_INT        {DIGIT}(_?{DIGIT})*
NUM_HEX        0[xX][0-9a-fA-F](_?[0-9a-fA-F])*
NUM_BIN        0[bB][01](_?[01])*
NUM_OCT        0[oO]?[0-7](_?[0-7])*
NUM_FLOAT      ({NUM_INT}\.{NUM_INT}?|\.{NUM_INT})([eE][+-]?{NUM_INT})?|{NUM_INT}[eE][+-]?{NUM_INT}

KEYWORD_JS     "var"|"let"|"const"|"function"|"return"|"if"|"else"|"for"|"while"|"do"|"switch"|"case"|"default"|"break"|"continue"|"new"|"this"|"class"|"extends"|"super"|"import"|"export"|"from"|"as"|"try"|"catch"|"finally"|"throw"|"async"|"await"|"yield"|"of"|"in"|"instanceof"|"typeof"|"void"|"delete"|"debugger"|"with"|"static"|"get"|"set"|"true"|"false"|"null"|"undefined"|"NaN"|"Infinity"

/* TypeScript-specific keywords */
KEYWORD_TS     "type"|"interface"|"enum"|"namespace"|"module"|"declare"|"abstract"|"implements"|"readonly"|"private"|"protected"|"public"|"keyof"|"infer"|"is"|"asserts"|"satisfies"|"override"|"const"|"enum"

/* Combined keywords */
KEYWORD        {KEYWORD_JS}|{KEYWORD_TS}

IDENTIFIER     [a-zA-Z_$][a-zA-Z0-9_$]*
COMMENT_SINGLE "//".*
COMMENT_MULTI  "/*"([^*]|"*"+[^*/])*"*"+"/"
STRING_DQ      "\""([^"\\\n]|\\.)*"\""
STRING_SQ      "'"([^'\\\n]|\\.)*"'"
STRING_BT      "`"([^`\\\n]|\\.)*"`"

/* JSX/TSX specific */
JSX_TAG_NAME   [a-zA-Z][a-zA-Z0-9_.-]*
JSX_ATTR       [a-zA-Z_:][a-zA-Z0-9_:]*
