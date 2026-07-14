import unittest
import os
from c_slides_bindings import CSlides


class TestLexerShell(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        lib_path = os.path.abspath(
            os.path.join(os.path.dirname(__file__), "../..", "libslider.so")
        )
        cls.cs = CSlides(lib_path)

    def setUp(self):
        self.theme_ptr = self.cs.theme_default()

    def _lex(self, line):
        return self.cs.highlight_lang(line, "bash", self.theme_ptr)

    def _assert_highlighted(self, out):
        self.assertIn("<span foreground=", out)

    # ── Comments ─────────────────────────────────────────────────────────

    def test_comment_simple(self):
        out = self._lex("# This is a comment")
        self.assertIn("comment", out)
        self._assert_highlighted(out)

    def test_comment_inline(self):
        out = self._lex("echo hello # inline comment")
        self.assertIn("comment", out)
        self._assert_highlighted(out)

    def test_comment_hashbang(self):
        out = self._lex("#!/bin/bash")
        self._assert_highlighted(out)

    def test_comment_at_end_of_line(self):
        out = self._lex("VAR=42 # set value")
        self.assertIn("42", out)
        self.assertIn("# set value", out)
        self._assert_highlighted(out)

    # ── Strings ──────────────────────────────────────────────────────────

    def test_string_double_quoted(self):
        out = self._lex('echo "Hello World"')
        self.assertIn("Hello World", out)
        self._assert_highlighted(out)

    def test_string_single_quoted(self):
        out = self._lex("echo 'literal string'")
        self.assertIn("literal string", out)
        self._assert_highlighted(out)

    def test_string_empty_double(self):
        out = self._lex('VAR=""')
        self._assert_highlighted(out)

    def test_string_empty_single(self):
        out = self._lex("VAR=''")
        self._assert_highlighted(out)

    def test_string_with_escaped_quote(self):
        out = self._lex(r'echo "say \"hi\""')
        self.assertIn("say", out)
        self._assert_highlighted(out)

    def test_string_with_variable(self):
        out = self._lex('echo "Home: $HOME"')
        self.assertIn("HOME", out)
        self._assert_highlighted(out)

    # ── Variables ────────────────────────────────────────────────────────

    def test_variable_simple(self):
        out = self._lex("echo $HOME")
        self.assertIn("HOME", out)
        self._assert_highlighted(out)

    def test_variable_with_braces(self):
        out = self._lex("echo ${NAME}")
        self.assertIn("NAME", out)
        self._assert_highlighted(out)

    def test_variable_with_default(self):
        out = self._lex("echo ${VAR:-default}")
        self.assertIn("VAR", out)
        self._assert_highlighted(out)

    def test_variable_with_assign(self):
        out = self._lex("echo ${VAR:=value}")
        self.assertIn("VAR", out)
        self._assert_highlighted(out)

    def test_variable_with_offset(self):
        out = self._lex("echo ${VAR:0:3}")
        self.assertIn("VAR", out)
        self._assert_highlighted(out)

    def test_variable_with_length(self):
        out = self._lex("echo ${#VAR}")
        self.assertIn("VAR", out)
        self._assert_highlighted(out)

    # ── Special variables ────────────────────────────────────────────────

    def test_special_var_question(self):
        out = self._lex("echo $?")
        self.assertIn("$?", out)
        self._assert_highlighted(out)

    def test_special_var_hash(self):
        out = self._lex("echo $#")
        self.assertIn("$#", out)
        self._assert_highlighted(out)

    def test_special_var_at(self):
        out = self._lex("echo $@")
        self.assertIn("$@", out)
        self._assert_highlighted(out)

    def test_special_var_star(self):
        out = self._lex("echo $*")
        self.assertIn("$*", out)
        self._assert_highlighted(out)

    def test_special_var_pid(self):
        out = self._lex("echo $$")
        self.assertIn("$$", out)
        self._assert_highlighted(out)

    # ── Positional parameters ────────────────────────────────────────────

    def test_positional_param_1(self):
        out = self._lex("echo $1")
        self.assertIn("$1", out)
        self._assert_highlighted(out)

    def test_positional_param_9(self):
        out = self._lex("echo $9")
        self.assertIn("$9", out)
        self._assert_highlighted(out)

    def test_positional_param_large(self):
        out = self._lex("echo $10")
        self.assertIn("$10", out)
        self._assert_highlighted(out)

    # ── Keywords ─────────────────────────────────────────────────────────

    def test_keyword_if(self):
        out = self._lex("if [ -f file ]; then")
        self.assertIn("if", out)
        self.assertIn("then", out)
        self._assert_highlighted(out)

    def test_keyword_elif(self):
        out = self._lex("elif [ -d dir ]; then")
        self.assertIn("elif", out)
        self._assert_highlighted(out)

    def test_keyword_else_fi(self):
        out = self._lex("else; fi")
        self.assertIn("else", out)
        self.assertIn("fi", out)
        self._assert_highlighted(out)

    def test_keyword_for_do_done(self):
        out = self._lex("for i in 1 2 3; do echo $i; done")
        self.assertIn("for", out)
        self.assertIn("do", out)
        self.assertIn("done", out)
        self._assert_highlighted(out)

    def test_keyword_while_do_done(self):
        out = self._lex("while true; do break; done")
        self.assertIn("while", out)
        self.assertIn("do", out)
        self.assertIn("done", out)
        self._assert_highlighted(out)

    def test_keyword_case_esac(self):
        out = self._lex("case $x in 1) echo a;; esac")
        self.assertIn("case", out)
        self.assertIn("esac", out)
        self.assertIn("in", out)
        self._assert_highlighted(out)

    def test_keyword_function(self):
        out = self._lex("function greet { echo hi; }")
        self.assertIn("function", out)
        self._assert_highlighted(out)

    def test_keyword_return(self):
        out = self._lex("return 0")
        self.assertIn("return", out)
        self._assert_highlighted(out)

    def test_keyword_exit(self):
        out = self._lex("exit 1")
        self.assertIn("exit", out)
        self._assert_highlighted(out)

    def test_keyword_break_continue(self):
        out = self._lex("break 2; continue")
        self.assertIn("break", out)
        self.assertIn("continue", out)
        self._assert_highlighted(out)

    def test_keyword_select(self):
        out = self._lex("select opt in a b c; do echo $opt; done")
        self.assertIn("select", out)
        self._assert_highlighted(out)

    def test_keyword_until(self):
        out = self._lex("until false; do sleep 1; done")
        self.assertIn("until", out)
        self._assert_highlighted(out)

    def test_keyword_time(self):
        out = self._lex("time ls")
        self.assertIn("time", out)
        self._assert_highlighted(out)

    # ── Builtins ─────────────────────────────────────────────────────────

    def test_builtin_echo(self):
        out = self._lex("echo hello")
        self.assertIn("echo", out)
        self._assert_highlighted(out)

    def test_builtin_printf(self):
        out = self._lex('printf "%s\\n" "hello"')
        self.assertIn("printf", out)
        self._assert_highlighted(out)

    def test_builtin_read(self):
        out = self._lex("read -r line")
        self.assertIn("read", out)
        self._assert_highlighted(out)

    def test_builtin_test(self):
        out = self._lex('test -f "file"')
        self.assertIn("test", out)
        self._assert_highlighted(out)

    def test_builtin_eval(self):
        out = self._lex('eval "echo hello"')
        self.assertIn("eval", out)
        self._assert_highlighted(out)

    def test_builtin_trap(self):
        out = self._lex('trap "cleanup" EXIT')
        self.assertIn("trap", out)
        self._assert_highlighted(out)

    def test_builtin_set(self):
        out = self._lex("set -euo pipefail")
        self.assertIn("set", out)
        self._assert_highlighted(out)

    def test_builtin_unset(self):
        out = self._lex("unset VAR")
        self.assertIn("unset", out)
        self._assert_highlighted(out)

    def test_builtin_export(self):
        out = self._lex('export PATH="$PATH:/new"')
        self.assertIn("export", out)
        self._assert_highlighted(out)

    def test_builtin_local(self):
        out = self._lex("local var=value")
        self.assertIn("local", out)
        self._assert_highlighted(out)

    def test_builtin_source(self):
        out = self._lex("source ./script.sh")
        self.assertIn("source", out)
        self._assert_highlighted(out)

    def test_builtin_alias(self):
        out = self._lex("alias ll='ls -la'")
        self.assertIn("alias", out)
        self._assert_highlighted(out)

    def test_builtin_cd(self):
        out = self._lex("cd /tmp")
        self.assertIn("cd", out)
        self._assert_highlighted(out)

    def test_builtin_shift(self):
        out = self._lex("shift 2")
        self.assertIn("shift", out)
        self._assert_highlighted(out)

    def test_builtin_typeset(self):
        out = self._lex("typeset -r VAR=42")
        self.assertIn("typeset", out)
        self._assert_highlighted(out)

    # ── Common commands ──────────────────────────────────────────────────

    def test_command_grep(self):
        out = self._lex('grep -r "pattern" .')
        self.assertIn("grep", out)
        self._assert_highlighted(out)

    def test_command_sed(self):
        out = self._lex('sed -i "s/old/new/g" file')
        self.assertIn("sed", out)
        self._assert_highlighted(out)

    def test_command_awk(self):
        out = self._lex("awk '{print $1}' file")
        self.assertIn("awk", out)
        self._assert_highlighted(out)

    def test_command_find(self):
        out = self._lex('find . -name "*.txt"')
        self.assertIn("find", out)
        self._assert_highlighted(out)

    def test_command_cat(self):
        out = self._lex("cat /etc/passwd")
        self.assertIn("cat", out)
        self._assert_highlighted(out)

    def test_command_ls(self):
        out = self._lex("ls -la /tmp")
        self.assertIn("ls", out)
        self._assert_highlighted(out)

    def test_command_cp(self):
        out = self._lex("cp file.txt /backup/")
        self.assertIn("cp", out)
        self._assert_highlighted(out)

    def test_command_mv(self):
        out = self._lex("mv old.txt new.txt")
        self.assertIn("mv", out)
        self._assert_highlighted(out)

    def test_command_rm(self):
        out = self._lex("rm -rf /tmp/build")
        self.assertIn("rm", out)
        self._assert_highlighted(out)

    def test_command_mkdir(self):
        out = self._lex("mkdir -p /path/to/dir")
        self.assertIn("mkdir", out)
        self._assert_highlighted(out)

    def test_command_chmod(self):
        out = self._lex("chmod 755 script.sh")
        self.assertIn("chmod", out)
        self._assert_highlighted(out)

    def test_command_curl(self):
        out = self._lex("curl -s https://example.com")
        self.assertIn("curl", out)
        self._assert_highlighted(out)

    def test_command_wget(self):
        out = self._lex("wget -q https://example.com/file.zip")
        self.assertIn("wget", out)
        self._assert_highlighted(out)

    def test_command_git(self):
        out = self._lex("git status")
        self.assertIn("git", out)
        self._assert_highlighted(out)

    def test_command_docker(self):
        out = self._lex("docker ps -a")
        self.assertIn("docker", out)
        self._assert_highlighted(out)

    def test_command_ssh(self):
        out = self._lex("ssh user@host")
        self.assertIn("ssh", out)
        self._assert_highlighted(out)

    # ── Operators ────────────────────────────────────────────────────────

    def test_operator_and(self):
        out = self._lex("a && b")
        self.assertIn("a", out)
        self.assertIn("b", out)
        self._assert_highlighted(out)

    def test_operator_or(self):
        out = self._lex("a || b")
        self.assertIn("a", out)
        self.assertIn("b", out)
        self._assert_highlighted(out)

    def test_operator_pipe(self):
        out = self._lex("ls | grep txt")
        self.assertIn("|", out)
        self._assert_highlighted(out)

    def test_operator_redirect_out(self):
        out = self._lex("echo hi > file.txt")
        self.assertIn(">", out)
        self._assert_highlighted(out)

    def test_operator_redirect_append(self):
        out = self._lex("echo hi >> file.txt")
        self.assertIn("echo", out)
        self.assertIn("file.txt", out)
        self._assert_highlighted(out)

    def test_operator_redirect_in(self):
        out = self._lex("sort < unsorted.txt")
        self.assertIn("<", out)
        self._assert_highlighted(out)

    def test_operator_redirect_stderr(self):
        out = self._lex("cmd &> log.txt")
        self.assertIn("cmd", out)
        self.assertIn("log.txt", out)
        self._assert_highlighted(out)

    def test_operator_heredoc(self):
        out = self._lex("cat <<EOF")
        self.assertIn("cat", out)
        self.assertIn("EOF", out)
        self._assert_highlighted(out)

    def test_operator_semicolon(self):
        out = self._lex("echo a; echo b")
        self.assertIn(";", out)
        self._assert_highlighted(out)

    def test_operator_background(self):
        out = self._lex("sleep 10 &")
        self.assertIn("&", out)
        self._assert_highlighted(out)

    def test_operator_not_equal(self):
        out = self._lex('[ "$a" != "$b" ]')
        self.assertIn("!", out)
        self._assert_highlighted(out)

    def test_operator_equal(self):
        out = self._lex('[ "$a" = "$b" ]')
        self._assert_highlighted(out)

    def test_operator_regex(self):
        out = self._lex('[[ "$str" =~ pattern ]]')
        self.assertIn("=~", out)
        self._assert_highlighted(out)

    # ── Variable assignments ─────────────────────────────────────────────

    def test_assignment_simple(self):
        out = self._lex("VAR=value")
        self.assertIn("VAR", out)
        self._assert_highlighted(out)

    def test_assignment_with_quotes(self):
        out = self._lex('VAR="hello world"')
        self.assertIn("VAR", out)
        self.assertIn("hello world", out)
        self._assert_highlighted(out)

    def test_assignment_with_command_substitution(self):
        out = self._lex("COUNT=$(wc -l < file.txt)")
        self.assertIn("COUNT", out)
        self.assertIn("wc", out)
        self._assert_highlighted(out)

    # ── Complex scripts ──────────────────────────────────────────────────

    def test_if_else_fi(self):
        out = self._lex('if [ -f "config.txt" ]; then source config.txt; else echo "No config"; fi')
        self.assertIn("if", out)
        self.assertIn("then", out)
        self.assertIn("source", out)
        self.assertIn("else", out)
        self.assertIn("echo", out)
        self.assertIn("fi", out)
        self._assert_highlighted(out)

    def test_for_loop(self):
        out = self._lex("for f in *.txt; do echo \"Processing $f\"; done")
        self.assertIn("for", out)
        self.assertIn("in", out)
        self.assertIn("do", out)
        self.assertIn("done", out)
        self._assert_highlighted(out)

    def test_while_read(self):
        out = self._lex('while IFS= read -r line; do echo "$line"; done < input.txt')
        self.assertIn("while", out)
        self.assertIn("read", out)
        self.assertIn("do", out)
        self.assertIn("done", out)
        self._assert_highlighted(out)

    def test_case_statement(self):
        out = self._lex('case "$1" in start) echo "Starting";; stop) echo "Stopping";; *) echo "Unknown";; esac')
        self.assertIn("case", out)
        self.assertIn("in", out)
        self.assertIn("esac", out)
        self._assert_highlighted(out)

    def test_function_definition(self):
        out = self._lex("greet() { echo \"Hello, $1!\"; }")
        self.assertIn("greet", out)
        self.assertIn("echo", out)
        self._assert_highlighted(out)

    def test_pipe_chain(self):
        out = self._lex("cat file.txt | sort | uniq -c | sort -rn | head -10")
        self.assertIn("cat", out)
        self.assertIn("sort", out)
        self.assertIn("uniq", out)
        self.assertIn("head", out)
        self._assert_highlighted(out)

    def test_lexer_dispatch_sh(self):
        out = self.cs.highlight_lang("echo hello", "sh", self.theme_ptr)
        self.assertIn("echo", out)
        self._assert_highlighted(out)

    def test_lexer_dispatch_bash(self):
        out = self.cs.highlight_lang("echo hello", "bash", self.theme_ptr)
        self.assertIn("echo", out)
        self._assert_highlighted(out)

    def test_lexer_dispatch_shell(self):
        out = self.cs.highlight_lang("echo hello", "shell", self.theme_ptr)
        self.assertIn("echo", out)
        self._assert_highlighted(out)

    def test_lexer_dispatch_fish(self):
        out = self.cs.highlight_lang("echo hello", "fish", self.theme_ptr)
        self.assertIn("echo", out)
        self._assert_highlighted(out)

    def test_lexer_dispatch_zsh(self):
        out = self.cs.highlight_lang("echo hello", "zsh", self.theme_ptr)
        self.assertIn("echo", out)
        self._assert_highlighted(out)

    def test_lexer_dispatch_env(self):
        out = self.cs.highlight_lang("echo hello", "env", self.theme_ptr)
        self.assertIn("echo", out)
        self._assert_highlighted(out)

    def test_empty_line(self):
        out = self._lex("")
        self.assertEqual(out, "")


if __name__ == "__main__":
    unittest.main()
