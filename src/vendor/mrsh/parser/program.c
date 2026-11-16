#define _POSIX_C_SOURCE 200809L
#include <assert.h>
#include <ctype.h>
#include <mrsh/buffer.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ast.h"
#include "parser.h"

static const char *operator_str(enum symbol_name sym) {
	for (size_t i = 0; i < operators_len; ++i) {
		if (operators[i].name == sym) {
			return operators[i].str;
		}
	}
	return NULL;
}

static bool operator(struct mrsh_parser *parser, enum symbol_name sym,
		struct mrsh_range *range) {
	if (!symbol(parser, sym)) {
		return false;
	}

	struct mrsh_position begin = parser->pos;

	const char *str = operator_str(sym);
	assert(str != NULL);

	char buf[operators_max_str_len];
	parser_read(parser, buf, strlen(str));
	assert(strncmp(str, buf, strlen(str)) == 0);

	if (range != NULL) {
		range->begin = begin;
		range->end = parser->pos;
	}

	consume_symbol(parser);
	return true;
}

static int separator_op(struct mrsh_parser *parser) {
	if (token(parser, "&", NULL)) {
		return '&';
	}
	if (token(parser, ";", NULL)) {
		return ';';
	}
	return -1;
}

static size_t peek_alias(struct mrsh_parser *parser) {
	size_t n = peek_word(parser, 0);

	for (size_t i = 0; i < n; ++i) {
		char c = parser->buf.data[i];
		switch (c) {
		case '_':
		case '!':
		case '%':
		case ',':
		case '@':
			break;
		default:
			if (!isalnum(c)) {
				return 0;
			}
		}
	}

	return n;
}

static void apply_aliases(struct mrsh_parser *parser) {
	if (parser->alias == NULL) {
		return;
	}

	const char *last_repl = NULL;
	while (true) {
		if (!symbol(parser, TOKEN)) {
			return;
		}

		size_t alias_len = peek_alias(parser);
		if (alias_len == 0) {
			return;
		}

		char *name = strndup(parser->buf.data, alias_len);
		if(!name){parser_oom(parser);return;}
		const char *repl = parser->alias(name, parser->alias_user_data);
		free(name);
		if (repl == NULL || last_repl == repl) {
			break;
		}

		size_t trailing_len = parser->buf.len - alias_len;
		size_t repl_len = strlen(repl);
		if (repl_len > alias_len) {
			if(!mrsh_buffer_reserve(&parser->buf,repl_len-alias_len)){parser_oom(parser);return;}
		}
		memmove(&parser->buf.data[repl_len], &parser->buf.data[alias_len],
			parser->buf.len - alias_len);
		memcpy(parser->buf.data, repl, repl_len);
		parser->buf.len = repl_len + trailing_len;

		// TODO: fixup parser->pos
		// TODO: if repl's last char is blank, replace next alias too

		consume_symbol(parser);
		last_repl = repl;
	}
}

static bool io_here(struct mrsh_parser *parser, struct mrsh_io_redirect *redir) {
	if (operator(parser, DLESS, &redir->op_range)) {
		redir->op = MRSH_IO_DLESS;
	} else if (operator(parser, DLESSDASH, &redir->op_range)) {
		redir->op = MRSH_IO_DLESSDASH;
	} else {
		return false;
	}

	redir->name = word(parser, 0);
	if (redir->name == NULL) {
		parser_set_error(parser,
			"expected a name after IO here-document redirection operator");
		return false;
	}
	// TODO: check redir->name only contains word strings and lists

	return true;
}

static struct mrsh_word *filename(struct mrsh_parser *parser) {
	// TODO: Apply rule 2
	return word(parser, 0);
}

static int io_redirect_op(struct mrsh_parser *parser, struct mrsh_range *range) {
	if (token(parser, "<", range)) {
		return MRSH_IO_LESS;
	} else if (token(parser, ">", range)) {
		return MRSH_IO_GREAT;
	} else if (operator(parser, LESSAND, range)) {
		return MRSH_IO_LESSAND;
	} else if (operator(parser, GREATAND, range)) {
		return MRSH_IO_GREATAND;
	} else if (operator(parser, DGREAT, range)) {
		return MRSH_IO_DGREAT;
	} else if (operator(parser, CLOBBER, range)) {
		return MRSH_IO_CLOBBER;
	} else if (operator(parser, LESSGREAT, range)) {
		return MRSH_IO_LESSGREAT;
	} else {
		return -1;
	}
}

static bool io_file(struct mrsh_parser *parser,
		struct mrsh_io_redirect *redir) {
	int op = io_redirect_op(parser, &redir->op_range);
	if (op < 0) {
		return false;
	}
	redir->op = op;

	redir->name = filename(parser);
	if (redir->name == NULL) {
		parser_set_error(parser,
			"expected a filename after IO file redirection operator");
		return false;
	}

	return true;
}

static int io_number(struct mrsh_parser *parser) {
	if (!symbol(parser, TOKEN)) {
		return -1;
	}

	char c = parser_peek_char(parser);
	if (!isdigit(c)) {
		return -1;
	}

	char buf[2];
	parser_peek(parser, buf, sizeof(buf));
	if (buf[1] != '<' && buf[1] != '>') {
		return -1;
	}

	parser_read_char(parser);
	consume_symbol(parser);
	return strtol(buf, NULL, 10);
}

static struct mrsh_io_redirect *io_redirect(struct mrsh_parser *parser) {
	struct mrsh_io_redirect redir = {0};

	struct mrsh_position io_number_pos = parser->pos;
	redir.io_number = io_number(parser);
	if (redir.io_number >= 0) {
		redir.io_number_pos = io_number_pos;
	}

	redir.op_range.begin = parser->pos;
	if (io_file(parser, &redir)) {
		struct mrsh_io_redirect *redir_ptr =
			calloc(1, sizeof(struct mrsh_io_redirect));
		if(!redir_ptr){mrsh_word_destroy(redir.name);parser_oom(parser);return NULL;}
		memcpy(redir_ptr, &redir, sizeof(struct mrsh_io_redirect));
		redir.op_range.end = parser->pos;
		return redir_ptr;
	}
	if (io_here(parser, &redir)) {
		struct mrsh_io_redirect *redir_ptr =
			calloc(1, sizeof(struct mrsh_io_redirect));
		if(!redir_ptr){mrsh_word_destroy(redir.name);parser_oom(parser);return NULL;}
		memcpy(redir_ptr, &redir, sizeof(struct mrsh_io_redirect));
		redir.op_range.end = parser->pos;
		if(mrsh_array_add(&parser->here_documents,redir_ptr)<0){mrsh_io_redirect_destroy(redir_ptr);parser_oom(parser);return NULL;}
		return redir_ptr;
	}

	if (redir.io_number >= 0) {
		parser_set_error(parser, "expected an IO redirect after IO number");
	}
	return NULL;
}

static struct mrsh_assignment *assignment_word(struct mrsh_parser *parser) {
	if (!symbol(parser, TOKEN)) {
		return NULL;
	}

	size_t name_len = peek_name(parser, false);
	if (name_len == 0) {
		return NULL;
	}

	if(parser_peek(parser,NULL,name_len+1)<name_len+1)return 0;
	if (parser->buf.data[name_len] != '=') {
		return NULL;
	}

	struct mrsh_range name_range;
	char *name = read_token(parser, name_len, &name_range);
	if(!name)return NULL;

	struct mrsh_position equal_pos = parser->pos;
	parser_read(parser, NULL, 1);

	/* Clarice: an empty assignment ends here; do not skip blanks into
	 * the following command while obtaining a fresh token. */
	parser->has_sym = true;
	parser->sym = TOKEN;
	struct mrsh_word *value = word(parser, 0);
	if (value == NULL) {
		if(mrsh_parser_error(parser,NULL)){free(name);return NULL;}
		struct mrsh_word_string *empty=parser_string(parser,strdup(""),false);
		if(!empty){free(name);return NULL;}value=&empty->word;
	}

	struct mrsh_assignment *assign = calloc(1, sizeof(struct mrsh_assignment));
	if(!assign){free(name);mrsh_word_destroy(value);parser_oom(parser);return NULL;}
	assign->name = name;
	assign->value = value;
	assign->name_range = name_range;
	assign->equal_pos = equal_pos;
	return assign;
}

static bool cmd_prefix(struct mrsh_parser *parser,
		struct mrsh_simple_command *cmd) {
	struct mrsh_io_redirect *redir = io_redirect(parser);
	if (redir != NULL) {
		if(mrsh_array_add(&cmd->io_redirects,redir)<0){mrsh_io_redirect_destroy(redir);parser_oom(parser);return false;}
		return true;
	}

	struct mrsh_assignment *assign = assignment_word(parser);
	if (assign != NULL) {
		if(mrsh_array_add(&cmd->assignments,assign)<0){mrsh_assignment_destroy(assign);parser_oom(parser);return false;}
		return true;
	}

	return false;
}

static struct mrsh_word *cmd_name(struct mrsh_parser *parser) {
	apply_aliases(parser);

	size_t word_len = peek_word(parser, 0);
	if (word_len == 0) {
		return word(parser, 0);
	}

	// TODO: optimize this
	for (size_t i = 0; i < keywords_len; ++i) {
		if (strlen(keywords[i]) == word_len &&
				strncmp(parser->buf.data, keywords[i], word_len) == 0) {
			return NULL;
		}
	}

	struct mrsh_range range;
	char *str = read_token(parser, word_len, &range);

	struct mrsh_word_string *ws = parser_string(parser,str,false);
	if(!ws)return NULL;
	ws->range = range;
	return &ws->word;
}

static bool cmd_suffix(struct mrsh_parser *parser,
		struct mrsh_simple_command *cmd) {
	struct mrsh_io_redirect *redir = io_redirect(parser);
	if (redir != NULL) {
		if(mrsh_array_add(&cmd->io_redirects,redir)<0){mrsh_io_redirect_destroy(redir);parser_oom(parser);return false;}
		return true;
	}

	struct mrsh_word *arg = word(parser, 0);
	if (arg != NULL) {
		if(!parser_word_add(parser,&cmd->arguments,arg))return false;
		return true;
	}

	return false;
}

static struct mrsh_simple_command *simple_command(struct mrsh_parser *parser) {
	struct mrsh_simple_command cmd = {0};

	bool has_prefix = false;
	while (cmd_prefix(parser, &cmd)) {
		has_prefix = true;
	}

	cmd.name = cmd_name(parser);
	if (cmd.name == NULL && !has_prefix) {
		redirect_array_finish(&cmd.io_redirects);assignment_array_finish(&cmd.assignments);return NULL;
	} else if (cmd.name != NULL) {
		while (cmd_suffix(parser, &cmd)) {
			// This space is intentionally left blank
		}
	}

	struct mrsh_simple_command *out=NULL;
 if(!mrsh_parser_error(parser,NULL))out=mrsh_simple_command_create(cmd.name,&cmd.arguments,&cmd.io_redirects,&cmd.assignments);
 if(!out){parser_oom(parser);mrsh_word_destroy(cmd.name);word_array_finish(&cmd.arguments);redirect_array_finish(&cmd.io_redirects);assignment_array_finish(&cmd.assignments);}return out;
}

static int separator(struct mrsh_parser *parser) {
	int sep = separator_op(parser);
	if (sep != -1) {
		linebreak(parser);
		return sep;
	}

	if (newline_list(parser)) {
		return '\n';
	}

	return -1;
}

static struct mrsh_and_or_list *and_or(struct mrsh_parser *parser);

static bool expect_here_document(struct mrsh_parser *parser,
	struct mrsh_io_redirect *redir, const char *delim);

static struct mrsh_command_list *term(struct mrsh_parser *parser) {
	struct mrsh_and_or_list *and_or_list = and_or(parser);
	if (and_or_list == NULL) {
		return NULL;
	}

	struct mrsh_command_list *cmd = mrsh_command_list_create();
	if(!cmd){mrsh_and_or_list_destroy(and_or_list);parser_oom(parser);return NULL;}
	cmd->and_or_list = and_or_list;

	struct mrsh_position separator_pos = parser->pos;
	int sep = separator(parser);
	if (sep == '&') {
		cmd->ampersand = true;
	}
	if (sep >= 0) {
		cmd->separator_pos = separator_pos;
	}

	if (sep == '\n' && parser->here_documents.len > 0) {
		for (size_t i = 0; i < parser->here_documents.len; ++i) {
			struct mrsh_io_redirect *redir = parser->here_documents.data[i];

			char *delim = mrsh_word_str(redir->name);
			bool ok=delim&&expect_here_document(parser,redir,delim);
			if(!delim)parser_oom(parser);
			free(delim);
			if (!ok) {
				mrsh_command_list_destroy(cmd);return NULL;
			}
		}

		parser->here_documents.len = 0;
	}

	return cmd;
}

static bool compound_list(struct mrsh_parser *parser, struct mrsh_array *cmds) {
	linebreak(parser);

	struct mrsh_command_list *l = term(parser);
	if (l == NULL) {
		return false;
	}
	if(mrsh_array_add(cmds,l)<0){mrsh_command_list_destroy(l);parser_oom(parser);return false;}

	while (true) {
		l = term(parser);
		if (l == NULL) {
			break;
		}
		if(mrsh_array_add(cmds,l)<0){mrsh_command_list_destroy(l);parser_oom(parser);return false;}
	}

	return true;
}

static bool expect_compound_list(struct mrsh_parser *parser,
		struct mrsh_array *cmds) {
	if (!compound_list(parser, cmds)) {
		command_list_array_finish(cmds);
		parser_set_error(parser, "expected a compound list");
		return false;
	}

	return true;
}

static struct mrsh_brace_group *brace_group(struct mrsh_parser *parser) {
	struct mrsh_position lbrace_pos = parser->pos;
	if (!token(parser, "{", NULL)) {
		return NULL;
	}

	struct mrsh_array body = {0};
	if (!expect_compound_list(parser, &body)) {
		return NULL;
	}

	struct mrsh_position rbrace_pos = parser->pos;
	if (!expect_token(parser, "}", NULL)) {
		command_list_array_finish(&body);
		return NULL;
	}

	struct mrsh_brace_group *bg = mrsh_brace_group_create(&body);
	if(!bg){command_list_array_finish(&body);parser_oom(parser);return NULL;}
	bg->lbrace_pos = lbrace_pos;
	bg->rbrace_pos = rbrace_pos;
	return bg;
}

static struct mrsh_subshell *subshell(struct mrsh_parser *parser) {
	struct mrsh_position lparen_pos = parser->pos;
	if (!token(parser, "(", NULL)) {
		return NULL;
	}

	struct mrsh_array body = {0};
	if (!expect_compound_list(parser, &body)) {
		return NULL;
	}

	struct mrsh_position rparen_pos = parser->pos;
	if (!expect_token(parser, ")", NULL)) {
		command_list_array_finish(&body);
		return NULL;
	}

	struct mrsh_subshell *s = mrsh_subshell_create(&body);
	if(!s){command_list_array_finish(&body);parser_oom(parser);return NULL;}
	s->lparen_pos = lparen_pos;
	s->rparen_pos = rparen_pos;
	return s;
}

static struct mrsh_command *else_part_inner(struct mrsh_parser *parser);
static struct mrsh_command *else_part(struct mrsh_parser *parser) {
 if (!parser_enter(parser)) return NULL;
 struct mrsh_command *result = else_part_inner(parser);
 parser_leave();
 return result;
}
static struct mrsh_command *else_part_inner(struct mrsh_parser *parser) {
	struct mrsh_range if_range = {0};
	if (token(parser, "elif", &if_range)) {
		struct mrsh_array cond = {0};
		if (!expect_compound_list(parser, &cond)) {
			return NULL;
		}

		struct mrsh_range then_range;
		if (!expect_token(parser, "then", &then_range)) {
			command_list_array_finish(&cond);
			return NULL;
		}

		struct mrsh_array body = {0};
		if (!expect_compound_list(parser, &body)) {
			command_list_array_finish(&cond);
			return NULL;
		}

		struct mrsh_command *ep = else_part(parser);

		struct mrsh_if_clause *ic = mrsh_if_clause_create(&cond, &body, ep);
	if(!ic){command_list_array_finish(&cond);command_list_array_finish(&body);mrsh_command_destroy(ep);parser_oom(parser);return NULL;}
		ic->if_range = if_range;
		ic->then_range = then_range;
		return &ic->command;
	}

	if (token(parser, "else", NULL)) {
		struct mrsh_array body = {0};
		if (!expect_compound_list(parser, &body)) {
			return NULL;
		}

		// TODO: position information is missing
		struct mrsh_brace_group *bg = mrsh_brace_group_create(&body);
	if(!bg){command_list_array_finish(&body);parser_oom(parser);return NULL;}
		return &bg->command;
	}

	return NULL;
}

static struct mrsh_if_clause *if_clause(struct mrsh_parser *parser) {
	struct mrsh_range if_range;
	if (!token(parser, "if", &if_range)) {
		return NULL;
	}

	struct mrsh_array cond = {0};
	if (!expect_compound_list(parser, &cond)) {
		goto error_cond;
	}

	struct mrsh_range then_range;
	if (!expect_token(parser, "then", &then_range)) {
		goto error_cond;
	}

	struct mrsh_array body = {0};
	if (!expect_compound_list(parser, &body)) {
		goto error_body;
	}

	struct mrsh_command *ep = else_part(parser);

	struct mrsh_range fi_range;
	if (!expect_token(parser, "fi", &fi_range)) {
		goto error_else_part;
	}

	struct mrsh_if_clause *ic = mrsh_if_clause_create(&cond, &body, ep);
	if(!ic){command_list_array_finish(&cond);command_list_array_finish(&body);mrsh_command_destroy(ep);parser_oom(parser);return NULL;}
	ic->if_range = if_range;
	ic->then_range = then_range;
	ic->fi_range = fi_range;
	return ic;

error_else_part:
	mrsh_command_destroy(ep);
error_body:
	command_list_array_finish(&body);
error_cond:
	command_list_array_finish(&cond);
	return NULL;
}

static bool sequential_sep(struct mrsh_parser *parser) {
	if (token(parser, ";", NULL)) {
		linebreak(parser);
		return true;
	}
	return newline_list(parser);
}

static void wordlist(struct mrsh_parser *parser,
		struct mrsh_array *words) {
	while (true) {
		struct mrsh_word *w = word(parser, 0);
		if (w == NULL) {
			break;
		}
		if(!parser_word_add(parser,words,w))return;
	}
}

static bool expect_do_group(struct mrsh_parser *parser,
		struct mrsh_array *body, struct mrsh_range *do_range,
		struct mrsh_range *done_range) {
	if (!token(parser, "do", do_range)) {
		parser_set_error(parser, "expected 'do'");
		return false;
	}

	if (!expect_compound_list(parser, body)) {
		return false;
	}

	if (!token(parser, "done", done_range)) {
		parser_set_error(parser, "expected 'done'");
		command_list_array_finish(body);
		return false;
	}

	return true;
}

static struct mrsh_for_clause *for_clause(struct mrsh_parser *parser) {
	struct mrsh_range for_range;
	if (!token(parser, "for", &for_range)) {
		return NULL;
	}

	size_t name_len = peek_name(parser, false);
	if (name_len == 0) {
		parser_set_error(parser, "expected name");
		return NULL;
	}

	struct mrsh_range name_range;
	char *name = read_token(parser, name_len, &name_range);
	if(!name)return NULL;

	linebreak(parser);

	struct mrsh_range in_range = {0};
	bool in = token(parser, "in", &in_range);

	// TODO: save sequential_sep position, if any
	struct mrsh_array words = {0};
	if (in) {
		wordlist(parser, &words);

		if (!sequential_sep(parser)) {
			parser_set_error(parser, "expected sequential separator");
			goto error_words;
		}
	} else {
		sequential_sep(parser);
	}

	struct mrsh_array body = {0};
	struct mrsh_range do_range, done_range;
	if (!expect_do_group(parser, &body, &do_range, &done_range)) {
		goto error_words;
	}

	struct mrsh_for_clause *fc =
		mrsh_for_clause_create(name, in, &words, &body);
	if(!fc){command_list_array_finish(&body);parser_oom(parser);goto error_words;}
	fc->for_range = for_range;
	fc->name_range = name_range;
	fc->in_range = in_range;
	fc->do_range = do_range;
	fc->done_range = done_range;
	return fc;

error_words:
	for (size_t i = 0; i < words.len; ++i) {
		struct mrsh_word *word = words.data[i];
		mrsh_word_destroy(word);
	}
	mrsh_array_finish(&words);
	free(name);
	return NULL;
}

static struct mrsh_loop_clause *loop_clause(struct mrsh_parser *parser) {
	enum mrsh_loop_type type;
	struct mrsh_range while_until_range;
	if (token(parser, "while", &while_until_range)) {
		type = MRSH_LOOP_WHILE;
	} else if (token(parser, "until", &while_until_range)) {
		type = MRSH_LOOP_UNTIL;
	} else {
		return NULL;
	}

	struct mrsh_array condition = {0};
	if (!expect_compound_list(parser, &condition)) {
		return NULL;
	}

	struct mrsh_array body = {0};
	struct mrsh_range do_range, done_range;
	if (!expect_do_group(parser, &body, &do_range, &done_range)) {
		command_list_array_finish(&condition);
		return NULL;
	}

	struct mrsh_loop_clause *fc =
		mrsh_loop_clause_create(type, &condition, &body);
	if(!fc){command_list_array_finish(&condition);command_list_array_finish(&body);parser_oom(parser);return NULL;}
	fc->while_until_range = while_until_range;
	fc->do_range = do_range;
	fc->done_range = done_range;
	return fc;
}

static struct mrsh_case_item *expect_case_item(struct mrsh_parser *parser,
		bool *dsemi) {
	struct mrsh_position lparen_pos = parser->pos;
	if (!token(parser, "(", NULL)) {
		lparen_pos = (struct mrsh_position){0};
	}

	struct mrsh_word *w = word(parser, 0);
	if (w == NULL) {
		parser_set_error(parser, "expected a word");
		return NULL;
	}

	struct mrsh_array patterns = {0};
	if(!parser_word_add(parser,&patterns,w))goto error_patterns;

	while (token(parser, "|", NULL)) {
		struct mrsh_word *w = word(parser, 0);
		if (w == NULL) {
			parser_set_error(parser, "expected a word");
			goto error_patterns;
		}
		if(!parser_word_add(parser,&patterns,w))goto error_patterns;
	}

	struct mrsh_position rparen_pos = parser->pos;
	if (!expect_token(parser, ")", NULL)) {
		goto error_patterns;
	}

	// It's okay if there's no body
	struct mrsh_array body = {0};
	compound_list(parser, &body);
	if (mrsh_parser_error(parser, NULL)) {
		goto error_body;
	}

	struct mrsh_range dsemi_range = {0};
	*dsemi = operator(parser, DSEMI, &dsemi_range);
	if (*dsemi) {
		linebreak(parser);
	}

	struct mrsh_case_item *item = calloc(1, sizeof(struct mrsh_case_item));
	if (item == NULL) {
		parser_oom(parser);goto error_body;
	}
	item->patterns = patterns;
	item->body = body;
	item->lparen_pos = lparen_pos;
	item->rparen_pos = rparen_pos;
	item->dsemi_range = dsemi_range;
	return item;

error_body:
	command_list_array_finish(&body);
error_patterns:
	for (size_t i = 0; i < patterns.len; ++i) {
		struct mrsh_word *w = patterns.data[i];
		mrsh_word_destroy(w);
	}
	mrsh_array_finish(&patterns);
	return NULL;
}

static struct mrsh_case_clause *case_clause(struct mrsh_parser *parser) {
	struct mrsh_range case_range;
	if (!token(parser, "case", &case_range)) {
		return NULL;
	}

	struct mrsh_word *w = word(parser, 0);
	if (w == NULL) {
		parser_set_error(parser, "expected a word");
		return NULL;
	}

	linebreak(parser);

	struct mrsh_range in_range;
	if (!expect_token(parser, "in", &in_range)) {
		goto error_word;
	}

	linebreak(parser);

	bool dsemi = false;
	struct mrsh_array items = {0};
	struct mrsh_range esac_range;
	while (!token(parser, "esac", &esac_range)) {
		struct mrsh_case_item *item = expect_case_item(parser, &dsemi);
		if (item == NULL) {
			goto error_items;
		}
		if(mrsh_array_add(&items,item)<0){case_item_destroy(item);parser_oom(parser);goto error_items;}

		if (!dsemi) {
			// Only the last case can omit `;;`
			if (!expect_token(parser, "esac", &esac_range)) {
				goto error_items;
			}
			break;
		}
	}

	struct mrsh_case_clause *cc = mrsh_case_clause_create(w, &items);
	if(!cc){parser_oom(parser);goto error_items;}
	cc->case_range = case_range;
	cc->in_range = in_range;
	cc->esac_range = esac_range;
	return cc;

error_items:
	for (size_t i = 0; i < items.len; ++i) {
		struct mrsh_case_item *item = items.data[i];
		case_item_destroy(item);
	}
	mrsh_array_finish(&items);
error_word:
	mrsh_word_destroy(w);
	return NULL;
}

static struct mrsh_command *compound_command(struct mrsh_parser *parser);

static struct mrsh_function_definition *function_definition(
		struct mrsh_parser *parser) {
	size_t name_len = peek_name(parser, false);
	if (name_len == 0) {
		return NULL;
	}

	size_t i = name_len;
	while (true) {
		if(parser_peek(parser,NULL,i+1)<i+1)return NULL;

		char c = parser->buf.data[i];
		if (c == '(') {
			break;
		} else if (!isblank(c)) {
			return NULL;
		}

		++i;
	}

	struct mrsh_range name_range;
	char *name = read_token(parser, name_len, &name_range);
	if(!name)return NULL;

	struct mrsh_position lparen_pos = parser->pos;
	if (!expect_token(parser, "(", NULL)) {
		free(name);return NULL;
	}

	struct mrsh_position rparen_pos = parser->pos;
	if (!expect_token(parser, ")", NULL)) {
		free(name);return NULL;
	}

	linebreak(parser);

	struct mrsh_command *cmd = compound_command(parser);
	if (cmd == NULL) {
		parser_set_error(parser, "expected a compound command");
		free(name);return NULL;
	}

	struct mrsh_array io_redirects = {0};
	while (true) {
		struct mrsh_io_redirect *redir = io_redirect(parser);
		if (redir == NULL) {
			break;
		}
		if(mrsh_array_add(&io_redirects,redir)<0){mrsh_io_redirect_destroy(redir);parser_oom(parser);break;}
	}

	struct mrsh_function_definition *fd=mrsh_parser_error(parser,NULL)?NULL:mrsh_function_definition_create(name,cmd,&io_redirects);
	if(!fd){free(name);mrsh_command_destroy(cmd);redirect_array_finish(&io_redirects);parser_oom(parser);return NULL;}
	fd->name_range = name_range;
	fd->lparen_pos = lparen_pos;
	fd->rparen_pos = rparen_pos;
	return fd;
}

static bool unspecified_word(struct mrsh_parser *parser) {
	const char *const reserved[] = {
		"[[",
		"]]",
		"function",
		"select",
	};

	size_t word_len = peek_word(parser, 0);
	if (word_len == 0) {
		return false;
	}

	for (size_t i = 0; i < sizeof(reserved) / sizeof(reserved[0]); i++) {
		if (strncmp(parser->buf.data, reserved[i], word_len) == 0 &&
				word_len == strlen(reserved[i])) {
			char err_msg[256];
			snprintf(err_msg, sizeof(err_msg),
				"keyword is reserved and causes unspecified results: %s",
				reserved[i]);
			parser_set_error(parser, err_msg);
			return true;
		}
	}

	size_t name_len = peek_name(parser, false);
	if (name_len == 0) {
		return false;
	}

	if(parser_peek(parser,NULL,name_len+1)<name_len+1)return 0;
	if (parser->buf.data[name_len] == ':') {
		parser_set_error(parser, "words that are the concatenation of a name "
			"and a colon produce unspecified results");
		return true;
	}

	return false;
}

static struct mrsh_command *compound_command_inner(struct mrsh_parser *parser);
static struct mrsh_command *compound_command(struct mrsh_parser *parser) {
 if (!parser_enter(parser)) return NULL;
 struct mrsh_command *result = compound_command_inner(parser);
 parser_leave();
 return result;
}
static struct mrsh_command *compound_command_inner(struct mrsh_parser *parser) {
	struct mrsh_brace_group *bg = brace_group(parser);
	if (bg != NULL) {
		return &bg->command;
	} else if (mrsh_parser_error(parser, NULL)) {
		return NULL;
	}

	struct mrsh_subshell *s = subshell(parser);
	if (s != NULL) {
		return &s->command;
	} else if (mrsh_parser_error(parser, NULL)) {
		return NULL;
	}

	struct mrsh_if_clause *ic = if_clause(parser);
	if (ic != NULL) {
		return &ic->command;
	} else if (mrsh_parser_error(parser, NULL)) {
		return NULL;
	}

	struct mrsh_for_clause *fc = for_clause(parser);
	if (fc != NULL) {
		return &fc->command;
	} else if (mrsh_parser_error(parser, NULL)) {
		return NULL;
	}

	struct mrsh_loop_clause *lc = loop_clause(parser);
	if (lc != NULL) {
		return &lc->command;
	} else if (mrsh_parser_error(parser, NULL)) {
		return NULL;
	}

	struct mrsh_case_clause *cc = case_clause(parser);
	if (cc != NULL) {
		return &cc->command;
	} else if (mrsh_parser_error(parser, NULL)) {
		return NULL;
	}

	if (unspecified_word(parser)) {
		return NULL;
	}

	struct mrsh_function_definition *fd = function_definition(parser);
	if (fd != NULL) {
		return &fd->command;
	} else if (mrsh_parser_error(parser, NULL)) {
		return NULL;
	}

	return NULL;
}

static struct mrsh_command *command(struct mrsh_parser *parser) {
	apply_aliases(parser);

	struct mrsh_command *cmd = compound_command(parser);
	if (cmd != NULL) {
		/* Clarice: scope each redirect to this complete compound command. */
		struct mrsh_io_redirect *redir;
		while ((redir = io_redirect(parser)) != NULL) {
			if (!cmd->redirects) cmd->redirects = calloc(1, sizeof *cmd->redirects);
			if (!cmd->redirects || mrsh_array_add(cmd->redirects, redir) < 0) {
				mrsh_io_redirect_destroy(redir);
				parser_set_error(parser, "out of memory for compound redirection");
				break;
			}
		}
		if (mrsh_parser_error(parser, NULL)) {mrsh_command_destroy(cmd);return NULL;}
		return cmd;
	} else if (mrsh_parser_error(parser, NULL)) return NULL;

	struct mrsh_simple_command *sc = simple_command(parser);
	if (sc != NULL) {
		return &sc->command;
	}

	return NULL;
}

static struct mrsh_pipeline *pipeline(struct mrsh_parser *parser) {
	struct mrsh_range bang_range = {0};
	bool bang = token(parser, "!", &bang_range);
	struct mrsh_position bang_pos = bang_range.begin; // can be invalid

	struct mrsh_command *cmd = command(parser);
	if (cmd == NULL) {
		return NULL;
	}

	struct mrsh_array commands = {0};
	if(mrsh_array_add(&commands,cmd)<0){mrsh_command_destroy(cmd);parser_oom(parser);goto error_commands;}

	while (token(parser, "|", NULL)) {
		linebreak(parser);
		struct mrsh_command *cmd = command(parser);
		if (cmd == NULL) {
			parser_set_error(parser, "expected a command");
			goto error_commands;
		}
		if(mrsh_array_add(&commands,cmd)<0){mrsh_command_destroy(cmd);parser_oom(parser);goto error_commands;}
	}

	struct mrsh_pipeline *p = mrsh_pipeline_create(&commands, bang);
	if(!p){parser_oom(parser);goto error_commands;}
	p->bang_pos = bang_pos;
	return p;

error_commands:
	for (size_t i = 0; i < commands.len; ++i) {
		mrsh_command_destroy((struct mrsh_command *)commands.data[i]);
	}
	mrsh_array_finish(&commands);
	return NULL;
}

/* POSIX AND-OR operators have equal precedence and associate left to right. */
static struct mrsh_and_or_list *and_or(struct mrsh_parser *parser) {
 struct mrsh_pipeline *first = pipeline(parser);
 if (!first) return NULL;
 struct mrsh_and_or_list *left = &first->and_or_list;
 unsigned int links=0;
 for (;;) {
  enum mrsh_binop_type op; struct mrsh_range range;
  if (operator(parser, AND_IF, &range)) op=MRSH_BINOP_AND;
  else if (operator(parser, OR_IF, &range)) op=MRSH_BINOP_OR;
  else return left;
  if (++links>64) { mrsh_and_or_list_destroy(left); parser_set_error(parser,"AND-OR tree depth limit (64)"); return NULL; }
  linebreak(parser);
  struct mrsh_pipeline *right=pipeline(parser);
  if (!right) {mrsh_and_or_list_destroy(left);parser_set_error(parser,"expected pipeline");return NULL;}
  struct mrsh_binop *bin=mrsh_binop_create(op,left,&right->and_or_list);
  if(!bin){mrsh_and_or_list_destroy(left);mrsh_and_or_list_destroy(&right->and_or_list);parser_oom(parser);return NULL;}
  bin->op_range=range;left=&bin->and_or_list;
 }
}

static struct mrsh_command_list *list(struct mrsh_parser *parser) {
	struct mrsh_and_or_list *and_or_list = and_or(parser);
	if (and_or_list == NULL) {
		return NULL;
	}

	struct mrsh_command_list *cmd = mrsh_command_list_create();
	if(!cmd){mrsh_and_or_list_destroy(and_or_list);parser_oom(parser);return NULL;}
	cmd->and_or_list = and_or_list;

	struct mrsh_position separator_pos = parser->pos;
	int sep = separator_op(parser);
	if (sep == '&') {
		cmd->ampersand = true;
	}
	if (sep >= 0) {
		cmd->separator_pos = separator_pos;
	}

	return cmd;
}

/**
 * Append a new string word to `children` with the contents of `buf`, and reset
 * `buf`.
 */
static bool push_buffer_word_string(struct mrsh_parser *parser,struct mrsh_array *children,
		struct mrsh_buffer *buf) {
	if (buf->len == 0) {
		return true;
	}

	if(!mrsh_buffer_append_char(buf,'\0')){parser_oom(parser);return false;}

	char *data = mrsh_buffer_steal(buf);
	struct mrsh_word_string *ws = parser_string(parser,data,false);
	if(!ws)return false;
	return parser_word_add(parser,children,&ws->word);
}

static struct mrsh_word *here_document_line(struct mrsh_parser *parser) {
	struct mrsh_array children = {0};
	struct mrsh_buffer buf = {0};

	while (true) {
		char c = parser_peek_char(parser);
		if (c == '\0') {
			break;
		}

		if (c == '$') {
			if(!push_buffer_word_string(parser,&children,&buf))goto error;
			struct mrsh_word *t = expect_dollar(parser);
			if (t == NULL) {
				goto error;
			}
			if(!parser_word_add(parser,&children,t))goto error;
			continue;
		}

		if (c == '`') {
			if(!push_buffer_word_string(parser,&children,&buf))goto error;
			struct mrsh_word *t = back_quotes(parser);
			if(!parser_word_add(parser,&children,t))goto error;
			continue;
		}

		if (c == '\\') {
			// Here-document backslash, same semantics as quoted backslash
			// except double-quotes are not special
			char next[2];
			parser_peek(parser, next, sizeof(next));
			switch (next[1]) {
			case '$':
			case '`':
			case '\\':
				parser_read_char(parser);
				c = next[1];
				break;
			}
		}

		parser_read_char(parser);
		if(!mrsh_buffer_append_char(&buf,c)){parser_oom(parser);goto error;}
	}

	if(!push_buffer_word_string(parser,&children,&buf))goto error;
	mrsh_buffer_finish(&buf);

	if (children.len == 1) {
		struct mrsh_word *word = children.data[0];
		mrsh_array_finish(&children); // TODO: don't allocate this array
		return word;
	} else {
		struct mrsh_word_list *wl = parser_words(parser,&children,false);
		if(!wl)goto error;
		return &wl->word;
	}
error:
	mrsh_buffer_finish(&buf);word_array_finish(&children);return NULL;
}

static bool is_word_quoted(struct mrsh_word *word) {
	switch (word->type) {
	case MRSH_WORD_STRING:;
		struct mrsh_word_string *ws = mrsh_word_get_string(word);
		return ws->single_quoted;
	case MRSH_WORD_LIST:;
		struct mrsh_word_list *wl = mrsh_word_get_list(word);
		if (wl->double_quoted) {
			return true;
		}
		for (size_t i = 0; i < wl->children.len; ++i) {
			struct mrsh_word *child = wl->children.data[i];
			if (is_word_quoted(child)) {
				return true;
			}
		}
		return false;
	default:
		abort();
	}
}

static bool expect_here_document(struct mrsh_parser *parser,
		struct mrsh_io_redirect *redir, const char *delim) {
	bool trim_tabs = redir->op == MRSH_IO_DLESSDASH;
	bool expand_lines = !is_word_quoted(redir->name);

	parser->continuation_line = true;

	struct mrsh_buffer buf = {0};
	while (true) {
		buf.len = 0;
		while (true) {
			char c = parser_peek_char(parser);
			if (c == '\0' || c == '\n') {
				break;
			}

			if(!mrsh_buffer_append_char(&buf,parser_read_char(parser))){parser_oom(parser);goto error;}
		}
		if(!mrsh_buffer_append_char(&buf,'\0')){parser_oom(parser);goto error;}

		const char *line = buf.data;
		if (trim_tabs) {
			while (line[0] == '\t') {
				++line;
			}
		}

		if (strcmp(line, delim) == 0) {
			if (parser_peek_char(parser) == '\n') {
				parser_read_char(parser);
			}
			break;
		}
		if (parser_peek_char(parser) == '\0') {
			parser_set_error(parser, "unterminated here-document");
			goto error;
		}
		read_continuation_line(parser);

		struct mrsh_word *word;
		if (expand_lines) {
			struct mrsh_parser *subparser =
				mrsh_parser_with_data(line, strlen(line));
			if(!subparser){parser_oom(parser);goto error;}
			word = here_document_line(subparser);
			if(mrsh_parser_error(subparser,NULL)){parser_set_error(parser,mrsh_parser_error(subparser,NULL));mrsh_word_destroy(word);word=NULL;}
			mrsh_parser_destroy(subparser);
		} else {
			struct mrsh_word_string *ws =
				parser_string(parser,strdup(line),true);
			word = ws?&ws->word:NULL;
		}

		if(!parser_word_add(parser,&redir->here_document,word))goto error;
	}
	mrsh_buffer_finish(&buf);

	consume_symbol(parser);
	return true;
error:
	mrsh_buffer_finish(&buf);return false;
}

static bool complete_command(struct mrsh_parser *parser,
		struct mrsh_array *cmds) {
	struct mrsh_command_list *l = list(parser);
	if (l == NULL) {
		return false;
	}
	if(mrsh_array_add(cmds,l)<0){mrsh_command_list_destroy(l);parser_oom(parser);return false;}

	while (true) {
		l = list(parser);
		if (l == NULL) {
			break;
		}
		if(mrsh_array_add(cmds,l)<0){mrsh_command_list_destroy(l);parser_oom(parser);return false;}
	}

	if (parser->here_documents.len > 0) {
		for (size_t i = 0; i < parser->here_documents.len; ++i) {
			struct mrsh_io_redirect *redir = parser->here_documents.data[i];

			if (!newline(parser)) {
				parser_set_error(parser,
					"expected a newline followed by a here-document");
				return false;
			}

			char *delim = mrsh_word_str(redir->name);
			bool ok=delim&&expect_here_document(parser,redir,delim);
			if(!delim)parser_oom(parser);
			free(delim);
			if (!ok) {
				return false;
			}
		}

		parser->here_documents.len = 0;
	}

	return true;
}

static bool expect_complete_command(struct mrsh_parser *parser,
		struct mrsh_array *cmds) {
	if (!complete_command(parser, cmds)) {
		parser_set_error(parser, "expected a complete command");
		return false;
	}

	return true;
}

static struct mrsh_program *program(struct mrsh_parser *parser) {
	struct mrsh_program *prog = mrsh_program_create();
	if (prog == NULL) {
		parser_oom(parser);return NULL;
	}

	linebreak(parser);
	if (eof(parser)) {
		return prog;
	}

	if (!expect_complete_command(parser, &prog->body)) {
		mrsh_program_destroy(prog);
		return NULL;
	}

	while (newline_list(parser)) {
		if (eof(parser)) {
			return prog;
		}

		if (!complete_command(parser, &prog->body)) {
			break;
		}
	}

	linebreak(parser);
	return prog;
}

struct mrsh_program *mrsh_parse_line(struct mrsh_parser *parser) {
	if(!parser)return NULL;
	parser_begin(parser);

	if (eof(parser)) {
		return NULL;
	}

	struct mrsh_program *prog = mrsh_program_create();
	if (prog == NULL) {
		parser_oom(parser);return NULL;
	}

	if (newline(parser)) {
		return prog;
	}

	if (!expect_complete_command(parser, &prog->body)) {
		goto error;
	}
	if (!eof(parser) && !newline(parser)) {
		parser_set_error(parser, "expected a newline");
		goto error;
	}

	return prog;

error:
	mrsh_program_destroy(prog);

	// Consume the whole line
	while (true) {
		char c = parser_peek_char(parser);
		if (c == '\0') {
			break;
		}

		parser_read_char(parser);
		if (c == '\n') {
			break;
		}
	}

	parser->has_sym = false;

	return NULL;
}

struct mrsh_program *mrsh_parse_program(struct mrsh_parser *parser) {
	if(!parser)return NULL;
	parser_begin(parser);
	struct mrsh_program *p=program(parser);
	if(mrsh_parser_error(parser,NULL)){mrsh_program_destroy(p);parser->here_documents.len=0;return NULL;}return p;
}
