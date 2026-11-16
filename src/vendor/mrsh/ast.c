#define _POSIX_C_SOURCE 200809L
#include <assert.h>
#include <mrsh/buffer.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "ast.h"

bool mrsh_position_valid(const struct mrsh_position *pos) {
	return pos->line > 0;
}

bool mrsh_range_valid(const struct mrsh_range *range) {
	return mrsh_position_valid(&range->begin) &&
		mrsh_position_valid(&range->end);
}

void mrsh_node_destroy(struct mrsh_node *node) {
	if (!node) return;
	switch (node->type) {
	case MRSH_NODE_PROGRAM:;
		struct mrsh_program *prog = mrsh_node_get_program(node);
		mrsh_program_destroy(prog);
		return;
	case MRSH_NODE_COMMAND_LIST:;
		struct mrsh_command_list *cl = mrsh_node_get_command_list(node);
		mrsh_command_list_destroy(cl);
		return;
	case MRSH_NODE_AND_OR_LIST:;
		struct mrsh_and_or_list *aol = mrsh_node_get_and_or_list(node);
		mrsh_and_or_list_destroy(aol);
		return;
	case MRSH_NODE_COMMAND:;
		struct mrsh_command *cmd = mrsh_node_get_command(node);
		mrsh_command_destroy(cmd);
		return;
	case MRSH_NODE_WORD:;
		struct mrsh_word *word = mrsh_node_get_word(node);
		mrsh_word_destroy(word);
		return;
	}
	abort();
}

void mrsh_word_destroy(struct mrsh_word *word) {
	if (word == NULL) {
		return;
	}

	switch (word->type) {
	case MRSH_WORD_STRING:;
		struct mrsh_word_string *ws = mrsh_word_get_string(word);
		free(ws->str);
		free(ws);
		return;
	case MRSH_WORD_PARAMETER:;
		struct mrsh_word_parameter *wp = mrsh_word_get_parameter(word);
		free(wp->name);
		mrsh_word_destroy(wp->arg);
		free(wp);
		return;
	case MRSH_WORD_COMMAND:;
		struct mrsh_word_command *wc = mrsh_word_get_command(word);
		mrsh_program_destroy(wc->program);
		free(wc);
		return;
	case MRSH_WORD_ARITHMETIC:;
		struct mrsh_word_arithmetic *wa = mrsh_word_get_arithmetic(word);
		mrsh_word_destroy(wa->body);
		free(wa);
		return;
	case MRSH_WORD_LIST:;
		struct mrsh_word_list *wl = mrsh_word_get_list(word);
		for (size_t i = 0; i < wl->children.len; ++i) {
			struct mrsh_word *child = wl->children.data[i];
			mrsh_word_destroy(child);
		}
		mrsh_array_finish(&wl->children);
		free(wl);
		return;
	}
	abort();
}

void mrsh_io_redirect_destroy(struct mrsh_io_redirect *redir) {
	if (redir == NULL) {
		return;
	}
	mrsh_word_destroy(redir->name);
	for (size_t i = 0; i < redir->here_document.len; ++i) {
		struct mrsh_word *line = redir->here_document.data[i];
		mrsh_word_destroy(line);
	}
	mrsh_array_finish(&redir->here_document);
	free(redir);
}

void mrsh_assignment_destroy(struct mrsh_assignment *assign) {
	if (assign == NULL) {
		return;
	}
	free(assign->name);
	mrsh_word_destroy(assign->value);
	free(assign);
}

void command_list_array_finish(struct mrsh_array *cmds) {
	for (size_t i = 0; i < cmds->len; ++i) {
		struct mrsh_command_list *l = cmds->data[i];
		mrsh_command_list_destroy(l);
	}
	mrsh_array_finish(cmds);
}

void case_item_destroy(struct mrsh_case_item *item) {
	for (size_t j = 0; j < item->patterns.len; ++j) {
		struct mrsh_word *pattern = item->patterns.data[j];
		mrsh_word_destroy(pattern);
	}
	mrsh_array_finish(&item->patterns);
	command_list_array_finish(&item->body);
	free(item);
}

void mrsh_command_destroy(struct mrsh_command *cmd) {
	if (cmd == NULL) {
		return;
	}

	if (cmd->redirects) {
		for (size_t i = 0; i < cmd->redirects->len; i++)
			mrsh_io_redirect_destroy(cmd->redirects->data[i]);
		mrsh_array_finish(cmd->redirects);
		free(cmd->redirects);
	}

	switch (cmd->type) {
	case MRSH_SIMPLE_COMMAND:;
		struct mrsh_simple_command *sc = mrsh_command_get_simple_command(cmd);
		mrsh_word_destroy(sc->name);
		for (size_t i = 0; i < sc->arguments.len; ++i) {
			struct mrsh_word *arg = sc->arguments.data[i];
			mrsh_word_destroy(arg);
		}
		mrsh_array_finish(&sc->arguments);
		for (size_t i = 0; i < sc->io_redirects.len; ++i) {
			struct mrsh_io_redirect *redir = sc->io_redirects.data[i];
			mrsh_io_redirect_destroy(redir);
		}
		mrsh_array_finish(&sc->io_redirects);
		for (size_t i = 0; i < sc->assignments.len; ++i) {
			struct mrsh_assignment *assign = sc->assignments.data[i];
			mrsh_assignment_destroy(assign);
		}
		mrsh_array_finish(&sc->assignments);
		free(sc);
		return;
	case MRSH_BRACE_GROUP:;
		struct mrsh_brace_group *bg = mrsh_command_get_brace_group(cmd);
		command_list_array_finish(&bg->body);
		free(bg);
		return;
	case MRSH_SUBSHELL:;
		struct mrsh_subshell *s = mrsh_command_get_subshell(cmd);
		command_list_array_finish(&s->body);
		free(s);
		return;
	case MRSH_IF_CLAUSE:;
		struct mrsh_if_clause *ic = mrsh_command_get_if_clause(cmd);
		command_list_array_finish(&ic->condition);
		command_list_array_finish(&ic->body);
		mrsh_command_destroy(ic->else_part);
		free(ic);
		return;
	case MRSH_FOR_CLAUSE:;
		struct mrsh_for_clause *fc = mrsh_command_get_for_clause(cmd);
		free(fc->name);
		for (size_t i = 0; i < fc->word_list.len; ++i) {
			struct mrsh_word *word = fc->word_list.data[i];
			mrsh_word_destroy(word);
		}
		mrsh_array_finish(&fc->word_list);
		command_list_array_finish(&fc->body);
		free(fc);
		return;
	case MRSH_LOOP_CLAUSE:;
		struct mrsh_loop_clause *lc = mrsh_command_get_loop_clause(cmd);
		command_list_array_finish(&lc->condition);
		command_list_array_finish(&lc->body);
		free(lc);
		return;
	case MRSH_CASE_CLAUSE:;
		struct mrsh_case_clause *cc = mrsh_command_get_case_clause(cmd);
		mrsh_word_destroy(cc->word);
		for (size_t i = 0; i < cc->items.len; ++i) {
			struct mrsh_case_item *item = cc->items.data[i];
			case_item_destroy(item);
		}
		mrsh_array_finish(&cc->items);
		free(cc);
		return;
	case MRSH_FUNCTION_DEFINITION:;
		struct mrsh_function_definition *fd =
			mrsh_command_get_function_definition(cmd);
		free(fd->name);
		mrsh_command_destroy(fd->body);
		for (size_t i = 0; i < fd->io_redirects.len; ++i) {
			struct mrsh_io_redirect *redir = fd->io_redirects.data[i];
			mrsh_io_redirect_destroy(redir);
		}
		mrsh_array_finish(&fd->io_redirects);
		free(fd);
		return;
	}
	abort();
}

void mrsh_and_or_list_destroy(struct mrsh_and_or_list *and_or_list) {
	if (and_or_list == NULL) {
		return;
	}

	switch (and_or_list->type) {
	case MRSH_AND_OR_LIST_PIPELINE:;
		struct mrsh_pipeline *p = mrsh_and_or_list_get_pipeline(and_or_list);
		for (size_t i = 0; i < p->commands.len; ++i) {
			struct mrsh_command *cmd = p->commands.data[i];
			mrsh_command_destroy(cmd);
		}
		mrsh_array_finish(&p->commands);
		free(p);
		return;
	case MRSH_AND_OR_LIST_BINOP:;
		struct mrsh_binop *binop = mrsh_and_or_list_get_binop(and_or_list);
		mrsh_and_or_list_destroy(binop->left);
		mrsh_and_or_list_destroy(binop->right);
		free(binop);
		return;
	}
	abort();
}

struct mrsh_command_list *mrsh_command_list_create(void) {
	struct mrsh_command_list *list = calloc(1, sizeof(struct mrsh_command_list));
	if (!list) return NULL;
	list->node.type = MRSH_NODE_COMMAND_LIST;
	return list;
}

void mrsh_command_list_destroy(struct mrsh_command_list *l) {
	if (l == NULL) {
		return;
	}

	mrsh_and_or_list_destroy(l->and_or_list);
	free(l);
}

struct mrsh_program *mrsh_program_create(void) {
	struct mrsh_program *prog = calloc(1, sizeof(struct mrsh_program));
	if (!prog) return NULL;
	prog->node.type = MRSH_NODE_PROGRAM;
	return prog;
}

void mrsh_program_destroy(struct mrsh_program *prog) {
	if (prog == NULL) {
		return;
	}

	command_list_array_finish(&prog->body);
	free(prog);
}

struct mrsh_word *mrsh_node_get_word(const struct mrsh_node *node) {
	assert(node->type == MRSH_NODE_WORD);
	return (struct mrsh_word *)node;
}

struct mrsh_command *mrsh_node_get_command(const struct mrsh_node *node) {
	assert(node->type == MRSH_NODE_COMMAND);
	return (struct mrsh_command *)node;
}

struct mrsh_and_or_list *mrsh_node_get_and_or_list(
		const struct mrsh_node *node) {
	assert(node->type == MRSH_NODE_AND_OR_LIST);
	return (struct mrsh_and_or_list *)node;
}

struct mrsh_command_list *mrsh_node_get_command_list(
		const struct mrsh_node *node) {
	assert(node->type == MRSH_NODE_COMMAND_LIST);
	return (struct mrsh_command_list *)node;
}

struct mrsh_program *mrsh_node_get_program(const struct mrsh_node *node) {
	assert(node->type == MRSH_NODE_PROGRAM);
	return (struct mrsh_program *)node;
}

struct mrsh_word_string *mrsh_word_string_create(char *str,
		bool single_quoted) {
	struct mrsh_word_string *ws = calloc(1, sizeof(struct mrsh_word_string));
	if (!ws) return NULL;
	ws->word.node.type = MRSH_NODE_WORD;
	ws->word.type = MRSH_WORD_STRING;
	ws->str = str;
	ws->single_quoted = single_quoted;
	return ws;
}

struct mrsh_word_parameter *mrsh_word_parameter_create(char *name,
		enum mrsh_word_parameter_op op, bool colon, struct mrsh_word *arg) {
	struct mrsh_word_parameter *wp =
		calloc(1, sizeof(struct mrsh_word_parameter));
	if (!wp) return NULL;
	wp->word.node.type = MRSH_NODE_WORD;
	wp->word.type = MRSH_WORD_PARAMETER;
	wp->name = name;
	wp->op = op;
	wp->colon = colon;
	wp->arg = arg;
	return wp;
}

struct mrsh_word_command *mrsh_word_command_create(struct mrsh_program *prog,
		bool back_quoted) {
	struct mrsh_word_command *wc =
		calloc(1, sizeof(struct mrsh_word_command));
	if (!wc) return NULL;
	wc->word.node.type = MRSH_NODE_WORD;
	wc->word.type = MRSH_WORD_COMMAND;
	wc->program = prog;
	wc->back_quoted = back_quoted;
	return wc;
}

struct mrsh_word_arithmetic *mrsh_word_arithmetic_create(
		struct mrsh_word *body) {
	struct mrsh_word_arithmetic *wa =
		calloc(1, sizeof(struct mrsh_word_arithmetic));
	if (!wa) return NULL;
	wa->word.node.type = MRSH_NODE_WORD;
	wa->word.type = MRSH_WORD_ARITHMETIC;
	wa->body = body;
	return wa;
}

struct mrsh_word_list *mrsh_word_list_create(struct mrsh_array *children,
		bool double_quoted) {
	struct mrsh_word_list *wl = calloc(1, sizeof(struct mrsh_word_list));
	if (!wl) return NULL;
	wl->word.node.type = MRSH_NODE_WORD;
	wl->word.type = MRSH_WORD_LIST;
	if (children != NULL) {
		wl->children = *children;
	}
	wl->double_quoted = double_quoted;
	return wl;
}

struct mrsh_word_string *mrsh_word_get_string(const struct mrsh_word *word) {
	assert(word->type == MRSH_WORD_STRING);
	return (struct mrsh_word_string *)word;
}

struct mrsh_word_parameter *mrsh_word_get_parameter(
		const struct mrsh_word *word) {
	assert(word->type == MRSH_WORD_PARAMETER);
	return (struct mrsh_word_parameter *)word;
}

struct mrsh_word_command *mrsh_word_get_command(const struct mrsh_word *word) {
	assert(word->type == MRSH_WORD_COMMAND);
	return (struct mrsh_word_command *)word;
}

struct mrsh_word_arithmetic *mrsh_word_get_arithmetic(
		const struct mrsh_word *word) {
	assert(word->type == MRSH_WORD_ARITHMETIC);
	return (struct mrsh_word_arithmetic *)word;
}

struct mrsh_word_list *mrsh_word_get_list(const struct mrsh_word *word) {
	assert(word->type == MRSH_WORD_LIST);
	return (struct mrsh_word_list *)word;
}

struct mrsh_simple_command *mrsh_simple_command_create(struct mrsh_word *name,
		struct mrsh_array *arguments, struct mrsh_array *io_redirects,
		struct mrsh_array *assignments) {
	struct mrsh_simple_command *cmd =
		calloc(1, sizeof(struct mrsh_simple_command));
	if (!cmd) return NULL;
	cmd->command.node.type = MRSH_NODE_COMMAND;
	cmd->command.type = MRSH_SIMPLE_COMMAND;
	cmd->name = name;
	cmd->arguments = *arguments;
	cmd->io_redirects = *io_redirects;
	cmd->assignments = *assignments;
	return cmd;
}

struct mrsh_brace_group *mrsh_brace_group_create(struct mrsh_array *body) {
	struct mrsh_brace_group *bg = calloc(1, sizeof(struct mrsh_brace_group));
	if (!bg) return NULL;
	bg->command.node.type = MRSH_NODE_COMMAND;
	bg->command.type = MRSH_BRACE_GROUP;
	bg->body = *body;
	return bg;
}

struct mrsh_subshell *mrsh_subshell_create(struct mrsh_array *body) {
	struct mrsh_subshell *s = calloc(1, sizeof(struct mrsh_subshell));
	if (!s) return NULL;
	s->command.node.type = MRSH_NODE_COMMAND;
	s->command.type = MRSH_SUBSHELL;
	s->body = *body;
	return s;
}

struct mrsh_if_clause *mrsh_if_clause_create(struct mrsh_array *condition,
		struct mrsh_array *body, struct mrsh_command *else_part) {
	struct mrsh_if_clause *ic = calloc(1, sizeof(struct mrsh_if_clause));
	if (!ic) return NULL;
	ic->command.node.type = MRSH_NODE_COMMAND;
	ic->command.type = MRSH_IF_CLAUSE;
	ic->condition = *condition;
	ic->body = *body;
	ic->else_part = else_part;
	return ic;
}

struct mrsh_for_clause *mrsh_for_clause_create(char *name, bool in,
		struct mrsh_array *word_list, struct mrsh_array *body) {
	struct mrsh_for_clause *fc = calloc(1, sizeof(struct mrsh_for_clause));
	if (!fc) return NULL;
	fc->command.node.type = MRSH_NODE_COMMAND;
	fc->command.type = MRSH_FOR_CLAUSE;
	fc->name = name;
	fc->in = in;
	fc->word_list = *word_list;
	fc->body = *body;
	return fc;
}

struct mrsh_loop_clause *mrsh_loop_clause_create(enum mrsh_loop_type type,
		struct mrsh_array *condition, struct mrsh_array *body) {
	struct mrsh_loop_clause *lc = calloc(1, sizeof(struct mrsh_loop_clause));
	if (!lc) return NULL;
	lc->command.node.type = MRSH_NODE_COMMAND;
	lc->command.type = MRSH_LOOP_CLAUSE;
	lc->type = type;
	lc->condition = *condition;
	lc->body = *body;
	return lc;
}

struct mrsh_case_clause *mrsh_case_clause_create(struct mrsh_word *word,
		struct mrsh_array *items) {
	struct mrsh_case_clause *cc = calloc(1, sizeof(struct mrsh_case_clause));
	if (!cc) return NULL;
	cc->command.node.type = MRSH_NODE_COMMAND;
	cc->command.type = MRSH_CASE_CLAUSE;
	cc->word = word;
	cc->items = *items;
	return cc;
}

struct mrsh_function_definition *mrsh_function_definition_create(char *name,
		struct mrsh_command *body, struct mrsh_array *io_redirects) {
	struct mrsh_function_definition *fd =
		calloc(1, sizeof(struct mrsh_function_definition));
	if (!fd) return NULL;
	fd->command.node.type = MRSH_NODE_COMMAND;
	fd->command.type = MRSH_FUNCTION_DEFINITION;
	fd->name = name;
	fd->body = body;
	fd->io_redirects = *io_redirects;
	return fd;
}

struct mrsh_simple_command *mrsh_command_get_simple_command(
		const struct mrsh_command *cmd) {
	assert(cmd->type == MRSH_SIMPLE_COMMAND);
	return (struct mrsh_simple_command *)cmd;
}

struct mrsh_brace_group *mrsh_command_get_brace_group(
		const struct mrsh_command *cmd) {
	assert(cmd->type == MRSH_BRACE_GROUP);
	return (struct mrsh_brace_group *)cmd;
}

struct mrsh_subshell *mrsh_command_get_subshell(
		const struct mrsh_command *cmd) {
	assert(cmd->type == MRSH_SUBSHELL);
	return (struct mrsh_subshell *)cmd;
}

struct mrsh_if_clause *mrsh_command_get_if_clause(
		const struct mrsh_command *cmd) {
	assert(cmd->type == MRSH_IF_CLAUSE);
	return (struct mrsh_if_clause *)cmd;
}

struct mrsh_for_clause *mrsh_command_get_for_clause(
		const struct mrsh_command *cmd) {
	assert(cmd->type == MRSH_FOR_CLAUSE);
	return (struct mrsh_for_clause *)cmd;
}

struct mrsh_loop_clause *mrsh_command_get_loop_clause(
		const struct mrsh_command *cmd) {
	assert(cmd->type == MRSH_LOOP_CLAUSE);
	return (struct mrsh_loop_clause *)cmd;
}

struct mrsh_case_clause *mrsh_command_get_case_clause(
		const struct mrsh_command *cmd) {
	assert(cmd->type == MRSH_CASE_CLAUSE);
	return (struct mrsh_case_clause *)cmd;
}

struct mrsh_function_definition *mrsh_command_get_function_definition(
		const struct mrsh_command *cmd) {
	assert(cmd->type == MRSH_FUNCTION_DEFINITION);
	return (struct mrsh_function_definition *)cmd;
}

struct mrsh_pipeline *mrsh_pipeline_create(struct mrsh_array *commands,
		bool bang) {
	struct mrsh_pipeline *pl = calloc(1, sizeof(struct mrsh_pipeline));
	if (!pl) return NULL;
	pl->and_or_list.node.type = MRSH_NODE_AND_OR_LIST;
	pl->and_or_list.type = MRSH_AND_OR_LIST_PIPELINE;
	pl->commands = *commands;
	pl->bang = bang;
	return pl;
}

struct mrsh_binop *mrsh_binop_create(enum mrsh_binop_type type,
		struct mrsh_and_or_list *left, struct mrsh_and_or_list *right) {
	struct mrsh_binop *binop = calloc(1, sizeof(struct mrsh_binop));
	if (!binop) return NULL;
	binop->and_or_list.node.type = MRSH_NODE_AND_OR_LIST;
	binop->and_or_list.type = MRSH_AND_OR_LIST_BINOP;
	binop->type = type;
	binop->left = left;
	binop->right = right;
	return binop;
}

struct mrsh_pipeline *mrsh_and_or_list_get_pipeline(
		const struct mrsh_and_or_list *and_or_list) {
	assert(and_or_list->type == MRSH_AND_OR_LIST_PIPELINE);
	return (struct mrsh_pipeline *)and_or_list;
}

struct mrsh_binop *mrsh_and_or_list_get_binop(
		const struct mrsh_and_or_list *and_or_list) {
	assert(and_or_list->type == MRSH_AND_OR_LIST_BINOP);
	return (struct mrsh_binop *)and_or_list;
}

static void node_array_for_each(struct mrsh_array *nodes,
		mrsh_node_iterator_func iterator, void *user_data) {
	for (size_t i = 0; i < nodes->len; ++i) {
		struct mrsh_node *node = nodes->data[i];
		mrsh_node_for_each(node, iterator, user_data);
	}
}

void mrsh_node_for_each(struct mrsh_node *node,
		mrsh_node_iterator_func iterator, void *user_data) {
	iterator(node, user_data);

	switch (node->type) {
	case MRSH_NODE_PROGRAM:;
		struct mrsh_program *program = mrsh_node_get_program(node);
		node_array_for_each(&program->body, iterator, user_data);
		return;
	case MRSH_NODE_COMMAND_LIST:;
		struct mrsh_command_list *list = mrsh_node_get_command_list(node);
		mrsh_node_for_each(&list->and_or_list->node, iterator, user_data);
		return;
	case MRSH_NODE_AND_OR_LIST:;
		struct mrsh_and_or_list *and_or_list = mrsh_node_get_and_or_list(node);
		switch (and_or_list->type) {
		case MRSH_AND_OR_LIST_BINOP:;
			struct mrsh_binop *binop = mrsh_and_or_list_get_binop(and_or_list);
			mrsh_node_for_each(&binop->left->node, iterator, user_data);
			mrsh_node_for_each(&binop->right->node, iterator, user_data);
			return;
		case MRSH_AND_OR_LIST_PIPELINE:;
			struct mrsh_pipeline *pipeline =
				mrsh_and_or_list_get_pipeline(and_or_list);
			node_array_for_each(&pipeline->commands, iterator, user_data);
			return;
		}
		abort();
	case MRSH_NODE_COMMAND:;
		struct mrsh_command *cmd = mrsh_node_get_command(node);
		switch (cmd->type) {
		case MRSH_SIMPLE_COMMAND:;
			struct mrsh_simple_command *sc =
				mrsh_command_get_simple_command(cmd);
			if (sc->name != NULL) {
				mrsh_node_for_each(&sc->name->node, iterator, user_data);
			}
			node_array_for_each(&sc->arguments, iterator, user_data);
			// TODO: io_redirects, assignments
			return;
		case MRSH_BRACE_GROUP:;
			struct mrsh_brace_group *bg = mrsh_command_get_brace_group(cmd);
			node_array_for_each(&bg->body, iterator, user_data);
			return;
		case MRSH_SUBSHELL:;
			struct mrsh_subshell *ss = mrsh_command_get_subshell(cmd);
			node_array_for_each(&ss->body, iterator, user_data);
			return;
		case MRSH_IF_CLAUSE:;
			struct mrsh_if_clause *ic = mrsh_command_get_if_clause(cmd);
			node_array_for_each(&ic->condition, iterator, user_data);
			node_array_for_each(&ic->body, iterator, user_data);
			if (ic->else_part != NULL) {
				mrsh_node_for_each(&ic->else_part->node, iterator, user_data);
			}
			return;
		case MRSH_FOR_CLAUSE:;
			struct mrsh_for_clause *fc = mrsh_command_get_for_clause(cmd);
			node_array_for_each(&fc->word_list, iterator, user_data);
			node_array_for_each(&fc->body, iterator, user_data);
			return;
		case MRSH_LOOP_CLAUSE:;
			struct mrsh_loop_clause *lc = mrsh_command_get_loop_clause(cmd);
			node_array_for_each(&lc->condition, iterator, user_data);
			node_array_for_each(&lc->body, iterator, user_data);
			return;
		case MRSH_CASE_CLAUSE:;
			struct mrsh_case_clause *cc = mrsh_command_get_case_clause(cmd);
			mrsh_node_for_each(&cc->word->node, iterator, user_data);
			// TODO: items
			return;
		case MRSH_FUNCTION_DEFINITION:;
			struct mrsh_function_definition *fn =
				mrsh_command_get_function_definition(cmd);
			mrsh_node_for_each(&fn->body->node, iterator, user_data);
			return;
		}
		abort();
	case MRSH_NODE_WORD:;
		struct mrsh_word *word = mrsh_node_get_word(node);
		switch (word->type) {
		case MRSH_WORD_STRING:
			return;
		case MRSH_WORD_PARAMETER:;
			struct mrsh_word_parameter *wp = mrsh_word_get_parameter(word);
			if (wp->arg != NULL) {
				mrsh_node_for_each(&wp->arg->node, iterator, user_data);
			}
			return;
		case MRSH_WORD_COMMAND:;
			struct mrsh_word_command *wc =	mrsh_word_get_command(word);
			if (wc->program != NULL) {
				mrsh_node_for_each(&wc->program->node, iterator, user_data);
			}
			return;
		case MRSH_WORD_ARITHMETIC:;
			struct mrsh_word_arithmetic *wa = mrsh_word_get_arithmetic(word);
			mrsh_node_for_each(&wa->body->node, iterator, user_data);
			return;
		case MRSH_WORD_LIST:;
			struct mrsh_word_list *wl = mrsh_word_get_list(word);
			node_array_for_each(&wl->children, iterator, user_data);
			return;
		}
		abort();
	}
	abort();
}

static void position_next(struct mrsh_position *dst,
		const struct mrsh_position *src) {
	*dst = *src;
	++dst->offset;
	++dst->column;
}

void mrsh_word_range(struct mrsh_word *word, struct mrsh_position *begin,
		struct mrsh_position *end) {
	if (begin == NULL && end == NULL) {
		return;
	}

	struct mrsh_position _begin, _end;
	if (begin == NULL) {
		begin = &_begin;
	}
	if (end == NULL) {
		end = &_end;
	}

	switch (word->type) {
	case MRSH_WORD_STRING:;
		struct mrsh_word_string *ws = mrsh_word_get_string(word);
		*begin = ws->range.begin;
		*end = ws->range.end;
		return;
	case MRSH_WORD_PARAMETER:;
		struct mrsh_word_parameter *wp = mrsh_word_get_parameter(word);
		*begin = wp->dollar_pos;
		if (mrsh_position_valid(&wp->rbrace_pos)) {
			position_next(end, &wp->rbrace_pos);
		} else {
			*end = wp->name_range.end;
		}
		return;
	case MRSH_WORD_COMMAND:;
		struct mrsh_word_command *wc = mrsh_word_get_command(word);
		*begin = wc->range.begin;
		*end = wc->range.end;
		return;
	case MRSH_WORD_ARITHMETIC:
		abort(); // TODO
	case MRSH_WORD_LIST:;
		struct mrsh_word_list *wl = mrsh_word_get_list(word);
		if (wl->children.len == 0) {
			*begin = *end = (struct mrsh_position){0};
		} else {
			struct mrsh_word *first = wl->children.data[0];
			struct mrsh_word *last = wl->children.data[wl->children.len - 1];
			mrsh_word_range(first, begin, NULL);
			mrsh_word_range(last, NULL, end);
		}
		return;
	}
	abort();
}

void mrsh_command_range(struct mrsh_command *cmd, struct mrsh_position *begin,
		struct mrsh_position *end) {
	if (begin == NULL && end == NULL) {
		return;
	}

	struct mrsh_position _begin, _end;
	if (begin == NULL) {
		begin = &_begin;
	}
	if (end == NULL) {
		end = &_end;
	}

	switch (cmd->type) {
	case MRSH_SIMPLE_COMMAND:;
		struct mrsh_simple_command *sc = mrsh_command_get_simple_command(cmd);

		if (sc->name != NULL) {
			mrsh_word_range(sc->name, begin, end);
		} else {
			assert(sc->assignments.len > 0);
			struct mrsh_assignment *first = sc->assignments.data[0];
			*begin = first->name_range.begin;
			*end = *begin; // That's a lie, but it'll be fixed by the code below
		}

		struct mrsh_position maybe_end;
		for (size_t i = 0; i < sc->arguments.len; ++i) {
			struct mrsh_word *arg = sc->arguments.data[i];
			mrsh_word_range(arg, NULL, &maybe_end);
			if (maybe_end.offset > end->offset) {
				*end = maybe_end;
			}
		}
		for (size_t i = 0; i < sc->io_redirects.len; ++i) {
			struct mrsh_io_redirect *redir = sc->io_redirects.data[i];
			mrsh_word_range(redir->name, NULL, &maybe_end);
			if (maybe_end.offset > end->offset) {
				*end = maybe_end;
			}
		}
		for (size_t i = 0; i < sc->assignments.len; ++i) {
			struct mrsh_assignment *assign = sc->assignments.data[i];
			mrsh_word_range(assign->value, NULL, &maybe_end);
			if (maybe_end.offset > end->offset) {
				*end = maybe_end;
			}
		}
		return;
	case MRSH_BRACE_GROUP:;
		struct mrsh_brace_group *bg = mrsh_command_get_brace_group(cmd);
		*begin = bg->lbrace_pos;
		position_next(end, &bg->rbrace_pos);
		return;
	case MRSH_SUBSHELL:;
		struct mrsh_subshell *s = mrsh_command_get_subshell(cmd);
		*begin = s->lparen_pos;
		position_next(end, &s->rparen_pos);
		return;
	case MRSH_IF_CLAUSE:;
		struct mrsh_if_clause *ic = mrsh_command_get_if_clause(cmd);
		*begin = ic->if_range.begin;
		*end = ic->fi_range.end;
		return;
	case MRSH_FOR_CLAUSE:;
		struct mrsh_for_clause *fc = mrsh_command_get_for_clause(cmd);
		*begin = fc->for_range.begin;
		*end = fc->done_range.end;
		return;
	case MRSH_LOOP_CLAUSE:;
		struct mrsh_loop_clause *lc = mrsh_command_get_loop_clause(cmd);
		*begin = lc->while_until_range.begin;
		*end = lc->done_range.end;
		return;
	case MRSH_CASE_CLAUSE:;
		struct mrsh_case_clause *cc = mrsh_command_get_case_clause(cmd);
		*begin = cc->case_range.begin;
		*end = cc->esac_range.end;
		return;
	case MRSH_FUNCTION_DEFINITION:;
		struct mrsh_function_definition *fd =
			mrsh_command_get_function_definition(cmd);
		*begin = fd->name_range.begin;
		mrsh_command_range(fd->body, NULL, end);
	}
	abort();
}

static void buffer_append_str(struct mrsh_buffer *buf, const char *str) {
	mrsh_buffer_append(buf, str, strlen(str));
}

static bool word_str(const struct mrsh_word *word, struct mrsh_buffer *buf) {
	switch (word->type) {
	case MRSH_WORD_STRING: {
		const char *str = mrsh_word_get_string(word)->str;
		return mrsh_buffer_append(buf, str, strlen(str));
	}
	case MRSH_WORD_LIST: {
		const struct mrsh_word_list *wl = mrsh_word_get_list(word);
		for (size_t i = 0; i < wl->children.len; ++i)
			if (!word_str(wl->children.data[i], buf)) return false;
		return true;
	}
	default:
		return false;
	}
}

char *mrsh_word_str(const struct mrsh_word *word) {
	struct mrsh_buffer buf = {0};
	if (!word_str(word, &buf) || !mrsh_buffer_append_char(&buf, '\0')) {
		mrsh_buffer_finish(&buf);
		return NULL;
	}
	return mrsh_buffer_steal(&buf);
}

static const char *binop_type_str(enum mrsh_binop_type t) {
	switch (t) {
	case MRSH_BINOP_AND:
		return "&&";
	case MRSH_BINOP_OR:
		return "||";
	}
	abort();
}

static void node_format(struct mrsh_node *node, struct mrsh_buffer *buf);

static void node_array_format(struct mrsh_array *array, const char *sep,
		struct mrsh_buffer *buf) {
	for (size_t i = 0; i < array->len; i++) {
		struct mrsh_node *node = array->data[i];
		if (i > 0) {
			buffer_append_str(buf, sep);
		}
		node_format(node, buf);
	}
}

static void node_format(struct mrsh_node *node, struct mrsh_buffer *buf) {
	switch (node->type) {
	case MRSH_NODE_PROGRAM:;
		struct mrsh_program *program = mrsh_node_get_program(node);
		node_array_format(&program->body, " ", buf);
		return;
	case MRSH_NODE_COMMAND_LIST:;
		struct mrsh_command_list *list = mrsh_node_get_command_list(node);
		node_format(&list->and_or_list->node, buf);
		buffer_append_str(buf, list->ampersand ? " &" : ";");
		return;
	case MRSH_NODE_AND_OR_LIST:;
		struct mrsh_and_or_list *and_or_list = mrsh_node_get_and_or_list(node);
		switch (and_or_list->type) {
		case MRSH_AND_OR_LIST_BINOP:;
			struct mrsh_binop *binop = mrsh_and_or_list_get_binop(and_or_list);
			node_format(&binop->left->node, buf);
			mrsh_buffer_append_char(buf, ' ');
			buffer_append_str(buf, binop_type_str(binop->type));
			mrsh_buffer_append_char(buf, ' ');
			node_format(&binop->right->node, buf);
			return;
		case MRSH_AND_OR_LIST_PIPELINE:;
			struct mrsh_pipeline *pipeline =
				mrsh_and_or_list_get_pipeline(and_or_list);
			if (pipeline->bang) {
				buffer_append_str(buf, "! ");
			}
			node_array_format(&pipeline->commands, " | ", buf);
			return;
		}
		abort();
	case MRSH_NODE_COMMAND:;
		struct mrsh_command *cmd = mrsh_node_get_command(node);
		switch (cmd->type) {
		case MRSH_SIMPLE_COMMAND:;
			struct mrsh_simple_command *sc =
				mrsh_command_get_simple_command(cmd);
			if (sc->name != NULL) {
				node_format(&sc->name->node, buf);
				mrsh_buffer_append_char(buf, ' ');
			}
			node_array_format(&sc->arguments, " ", buf);
			// TODO: io_redirects, assignments
			return;
		case MRSH_BRACE_GROUP:;
			struct mrsh_brace_group *bg = mrsh_command_get_brace_group(cmd);
			buffer_append_str(buf, "{ ");
			node_array_format(&bg->body, " ", buf);
			buffer_append_str(buf, "; }");
			return;
		case MRSH_SUBSHELL:;
			struct mrsh_subshell *ss = mrsh_command_get_subshell(cmd);
			mrsh_buffer_append_char(buf, '(');
			node_array_format(&ss->body, " ", buf);
			mrsh_buffer_append_char(buf, ')');
			return;
		case MRSH_IF_CLAUSE:;
			struct mrsh_if_clause *ic = mrsh_command_get_if_clause(cmd);
			buffer_append_str(buf, "if ");
			node_array_format(&ic->condition, " ", buf);
			buffer_append_str(buf, "then ");
			node_array_format(&ic->body, " ", buf);
			if (ic->else_part != NULL) {
				// TODO: elif
				buffer_append_str(buf, "else ");
				node_format(&ic->else_part->node, buf);
			}
			buffer_append_str(buf, "fi");
			return;
		case MRSH_FOR_CLAUSE:;
			//struct mrsh_for_clause *fc = mrsh_command_get_for_clause(cmd);
			// TODO
			return;
		case MRSH_LOOP_CLAUSE:;
			struct mrsh_loop_clause *lc = mrsh_command_get_loop_clause(cmd);
			buffer_append_str(buf,
				lc->type == MRSH_LOOP_WHILE ? "while " : "until ");
			node_array_format(&lc->condition, " ", buf);
			buffer_append_str(buf, "do ");
			node_array_format(&lc->body, " ", buf);
			buffer_append_str(buf, "done");
			return;
		case MRSH_CASE_CLAUSE:;
			//struct mrsh_case_clause *cc = mrsh_command_get_case_clause(cmd);
			// TODO
			return;
		case MRSH_FUNCTION_DEFINITION:;
			struct mrsh_function_definition *fn =
				mrsh_command_get_function_definition(cmd);
			buffer_append_str(buf, fn->name);
			buffer_append_str(buf, "()");
			node_format(&fn->body->node, buf);
			// TODO: io-redirect
			return;
		}
		abort();
	case MRSH_NODE_WORD:;
		// TODO: quoting
		struct mrsh_word *word = mrsh_node_get_word(node);
		switch (word->type) {
		case MRSH_WORD_STRING:;
			struct mrsh_word_string *ws = mrsh_word_get_string(word);
			if (ws->single_quoted) {
				mrsh_buffer_append_char(buf, '\'');
			}
			buffer_append_str(buf, ws->str);
			if (ws->single_quoted) {
				mrsh_buffer_append_char(buf, '\'');
			}
			return;
		case MRSH_WORD_PARAMETER:;
			struct mrsh_word_parameter *wp = mrsh_word_get_parameter(word);
			buffer_append_str(buf, "${");
			if (wp->arg != NULL) {
				node_format(&wp->arg->node, buf);
			}
			buffer_append_str(buf, "}");
			return;
		case MRSH_WORD_COMMAND:;
			struct mrsh_word_command *wc =	mrsh_word_get_command(word);
			buffer_append_str(buf, wc->back_quoted ? "`" : "$(");
			if (wc->program != NULL) {
				node_format(&wc->program->node, buf);
			}
			buffer_append_str(buf, wc->back_quoted ? "`" : ")");
			return;
		case MRSH_WORD_ARITHMETIC:;
			struct mrsh_word_arithmetic *wa = mrsh_word_get_arithmetic(word);
			node_format(&wa->body->node, buf);
			return;
		case MRSH_WORD_LIST:;
			struct mrsh_word_list *wl = mrsh_word_get_list(word);
			if (wl->double_quoted) {
				mrsh_buffer_append_char(buf, '"');
			}
			node_array_format(&wl->children, "", buf);
			if (wl->double_quoted) {
				mrsh_buffer_append_char(buf, '"');
			}
			return;
		}
		abort();
	}
	abort();
}

char *mrsh_node_format(struct mrsh_node *node) {
	struct mrsh_buffer buf = {0};
	node_format(node, &buf);
	mrsh_buffer_append_char(&buf, '\0');
	return mrsh_buffer_steal(&buf);
}

/* Copies preserve source positions and own only fully constructed children.
 * Failure destroys the partial destination, never the source. */
static struct mrsh_case_item *case_item_copy(const struct mrsh_case_item *);
static bool copy_words(struct mrsh_array *d,const struct mrsh_array *s) {
	if(!mrsh_array_reserve(d,s->len))return false;
	for(size_t i=0;i<s->len;i++) {
		struct mrsh_word *p=mrsh_word_copy(s->data[i]);
		if(!p)return false;
		d->data[d->len++]=p;
	}
	return true;
}
static bool copy_redirects(struct mrsh_array *d,const struct mrsh_array *s) {
	if(!mrsh_array_reserve(d,s->len))return false;
	for(size_t i=0;i<s->len;i++) {
		struct mrsh_io_redirect *p=mrsh_io_redirect_copy(s->data[i]);
		if(!p)return false;
		d->data[d->len++]=p;
	}
	return true;
}
static bool copy_assignments(struct mrsh_array *d,const struct mrsh_array *s) {
	if(!mrsh_array_reserve(d,s->len))return false;
	for(size_t i=0;i<s->len;i++) {
		struct mrsh_assignment *p=mrsh_assignment_copy(s->data[i]);
		if(!p)return false;
		d->data[d->len++]=p;
	}
	return true;
}
static bool copy_lists(struct mrsh_array *d,const struct mrsh_array *s) {
	if(!mrsh_array_reserve(d,s->len))return false;
	for(size_t i=0;i<s->len;i++) {
		struct mrsh_command_list *p=mrsh_command_list_copy(s->data[i]);
		if(!p)return false;
		d->data[d->len++]=p;
	}
	return true;
}
static bool copy_commands(struct mrsh_array *d,const struct mrsh_array *s) {
	if(!mrsh_array_reserve(d,s->len))return false;
	for(size_t i=0;i<s->len;i++) {
		struct mrsh_command *p=mrsh_command_copy(s->data[i]);
		if(!p)return false;
		d->data[d->len++]=p;
	}
	return true;
}
static bool copy_cases(struct mrsh_array *d,const struct mrsh_array *s) {
	if(!mrsh_array_reserve(d,s->len))return false;
	for(size_t i=0;i<s->len;i++) {
		struct mrsh_case_item *p=case_item_copy(s->data[i]);
		if(!p)return false;
		d->data[d->len++]=p;
	}
	return true;
}
struct mrsh_node *mrsh_node_copy(const struct mrsh_node *n) {
	if(!n)return NULL;
	switch(n->type) {
		case MRSH_NODE_PROGRAM:return (struct mrsh_node*)mrsh_program_copy((const struct mrsh_program*)n);
		case MRSH_NODE_COMMAND_LIST:return (struct mrsh_node*)mrsh_command_list_copy((const struct mrsh_command_list*)n);
		case MRSH_NODE_AND_OR_LIST:return (struct mrsh_node*)mrsh_and_or_list_copy((const struct mrsh_and_or_list*)n);
		case MRSH_NODE_COMMAND:return (struct mrsh_node*)mrsh_command_copy((const struct mrsh_command*)n);
		case MRSH_NODE_WORD:return (struct mrsh_node*)mrsh_word_copy((const struct mrsh_word*)n);
	}
	return NULL;
}
struct mrsh_io_redirect *mrsh_io_redirect_copy(const struct mrsh_io_redirect *s) {
	if(!s)return NULL;
	struct mrsh_io_redirect *d=calloc(1,sizeof *d);
	if(!d)return NULL;
	*d=*s;
	d->name=NULL;
	d->here_document=(struct mrsh_array) {
		0
	};
	if(s->name && !(d->name=mrsh_word_copy(s->name)))goto error;
	if(!copy_words(&d->here_document,&s->here_document))goto error;
	return d;
	error:mrsh_io_redirect_destroy(d);
	return NULL;
}
struct mrsh_assignment *mrsh_assignment_copy(const struct mrsh_assignment *s) {
	if(!s)return NULL;
	struct mrsh_assignment *d=calloc(1,sizeof *d);
	if(!d)return NULL;
	*d=*s;
	d->name=NULL;
	d->value=NULL;
	if(s->name && !(d->name=strdup(s->name)))goto error;
	if(s->value && !(d->value=mrsh_word_copy(s->value)))goto error;
	return d;
	error:mrsh_assignment_destroy(d);
	return NULL;
}
static struct mrsh_case_item *case_item_copy(const struct mrsh_case_item *s) {
	if(!s)return NULL;
	struct mrsh_case_item *d=calloc(1,sizeof *d);
	if(!d)return NULL;
	*d=*s;
	d->patterns=(struct mrsh_array) {
		0
	};
	d->body=(struct mrsh_array) {
		0
	};
	if(!copy_words(&d->patterns,&s->patterns))goto error;
	if(!copy_lists(&d->body,&s->body))goto error;
	return d;
	error:case_item_destroy(d);
	return NULL;
}
struct mrsh_command_list *mrsh_command_list_copy(const struct mrsh_command_list *s) {
	if(!s)return NULL;
	struct mrsh_command_list *d=calloc(1,sizeof *d);
	if(!d)return NULL;
	*d=*s;
	d->and_or_list=NULL;
	if(s->and_or_list && !(d->and_or_list=mrsh_and_or_list_copy(s->and_or_list)))goto error;
	return d;
	error:mrsh_command_list_destroy(d);
	return NULL;
}
struct mrsh_program *mrsh_program_copy(const struct mrsh_program *s) {
	if(!s)return NULL;
	struct mrsh_program *d=calloc(1,sizeof *d);
	if(!d)return NULL;
	*d=*s;
	d->body=(struct mrsh_array) {
		0
	};
	if(!copy_lists(&d->body,&s->body))goto error;
	return d;
	error:mrsh_program_destroy(d);
	return NULL;
}
struct mrsh_word *mrsh_word_copy(const struct mrsh_word *w) {
	if(!w)return NULL;
	switch(w->type) {
		case MRSH_WORD_STRING: {
			const struct mrsh_word_string *s=(const void*)w;
			struct mrsh_word_string *d=calloc(1,sizeof *d);
			if(!d)return NULL;
			*d=*s;
			d->str=NULL;
			if(s->str && !(d->str=strdup(s->str)))goto error_STRING;
			return &d->word;
			error_STRING:mrsh_word_destroy(&d->word);
			return NULL;
		}
		case MRSH_WORD_PARAMETER: {
			const struct mrsh_word_parameter *s=(const void*)w;
			struct mrsh_word_parameter *d=calloc(1,sizeof *d);
			if(!d)return NULL;
			*d=*s;
			d->name=NULL;
			d->arg=NULL;
			if(s->name && !(d->name=strdup(s->name)))goto error_PARAMETER;
			if(s->arg && !(d->arg=mrsh_word_copy(s->arg)))goto error_PARAMETER;
			return &d->word;
			error_PARAMETER:mrsh_word_destroy(&d->word);
			return NULL;
		}
		case MRSH_WORD_COMMAND: {
			const struct mrsh_word_command *s=(const void*)w;
			struct mrsh_word_command *d=calloc(1,sizeof *d);
			if(!d)return NULL;
			*d=*s;
			d->program=NULL;
			if(s->program && !(d->program=mrsh_program_copy(s->program)))goto error_COMMAND;
			return &d->word;
			error_COMMAND:mrsh_word_destroy(&d->word);
			return NULL;
		}
		case MRSH_WORD_ARITHMETIC: {
			const struct mrsh_word_arithmetic *s=(const void*)w;
			struct mrsh_word_arithmetic *d=calloc(1,sizeof *d);
			if(!d)return NULL;
			*d=*s;
			d->body=NULL;
			if(s->body && !(d->body=mrsh_word_copy(s->body)))goto error_ARITHMETIC;
			return &d->word;
			error_ARITHMETIC:mrsh_word_destroy(&d->word);
			return NULL;
		}
		case MRSH_WORD_LIST: {
			const struct mrsh_word_list *s=(const void*)w;
			struct mrsh_word_list *d=calloc(1,sizeof *d);
			if(!d)return NULL;
			*d=*s;
			d->children=(struct mrsh_array) {
				0
			};
			if(!copy_words(&d->children,&s->children))goto error_LIST;
			return &d->word;
			error_LIST:mrsh_word_destroy(&d->word);
			return NULL;
		}
	}
	return NULL;
}
struct mrsh_and_or_list *mrsh_and_or_list_copy(const struct mrsh_and_or_list *a) {
	if(!a)return NULL;
	switch(a->type) {
		case MRSH_AND_OR_LIST_PIPELINE: {
			const struct mrsh_pipeline *s=(const void*)a;
			struct mrsh_pipeline *d=calloc(1,sizeof *d);
			if(!d)return NULL;
			*d=*s;
			d->commands=(struct mrsh_array) {
				0
			};
			if(!copy_commands(&d->commands,&s->commands))goto error_PIPELINE;
			return &d->and_or_list;
			error_PIPELINE:mrsh_and_or_list_destroy(&d->and_or_list);
			return NULL;
		}
		case MRSH_AND_OR_LIST_BINOP: {
			const struct mrsh_binop *s=(const void*)a;
			struct mrsh_binop *d=calloc(1,sizeof *d);
			if(!d)return NULL;
			*d=*s;
			d->left=NULL;
			d->right=NULL;
			if(s->left && !(d->left=mrsh_and_or_list_copy(s->left)))goto error_BINOP;
			if(s->right && !(d->right=mrsh_and_or_list_copy(s->right)))goto error_BINOP;
			return &d->and_or_list;
			error_BINOP:mrsh_and_or_list_destroy(&d->and_or_list);
			return NULL;
		}
	}
	return NULL;
}
struct mrsh_command *mrsh_command_copy(const struct mrsh_command *c) {
	if(!c)return NULL;
	struct mrsh_command *result=NULL;
	switch(c->type) {
		case MRSH_SIMPLE_COMMAND: {
			const struct mrsh_simple_command *s=(const void*)c;
			struct mrsh_simple_command *d=calloc(1,sizeof *d);
			if(!d)return NULL;
			*d=*s;
			d->name=NULL;
			d->arguments=(struct mrsh_array) {
				0
			};
			d->io_redirects=(struct mrsh_array) {
				0
			};
			d->assignments=(struct mrsh_array) {
				0
			};
			d->command.redirects=NULL;
			if(s->name && !(d->name=mrsh_word_copy(s->name)))goto error_SIMPLE_COMMAND;
			if(!copy_words(&d->arguments,&s->arguments))goto error_SIMPLE_COMMAND;
			if(!copy_redirects(&d->io_redirects,&s->io_redirects))goto error_SIMPLE_COMMAND;
			if(!copy_assignments(&d->assignments,&s->assignments))goto error_SIMPLE_COMMAND;
			result=&d->command;
			break;
			error_SIMPLE_COMMAND:mrsh_command_destroy(&d->command);
			return NULL;
		}
		case MRSH_BRACE_GROUP: {
			const struct mrsh_brace_group *s=(const void*)c;
			struct mrsh_brace_group *d=calloc(1,sizeof *d);
			if(!d)return NULL;
			*d=*s;
			d->body=(struct mrsh_array) {
				0
			};
			d->command.redirects=NULL;
			if(!copy_lists(&d->body,&s->body))goto error_BRACE_GROUP;
			result=&d->command;
			break;
			error_BRACE_GROUP:mrsh_command_destroy(&d->command);
			return NULL;
		}
		case MRSH_SUBSHELL: {
			const struct mrsh_subshell *s=(const void*)c;
			struct mrsh_subshell *d=calloc(1,sizeof *d);
			if(!d)return NULL;
			*d=*s;
			d->body=(struct mrsh_array) {
				0
			};
			d->command.redirects=NULL;
			if(!copy_lists(&d->body,&s->body))goto error_SUBSHELL;
			result=&d->command;
			break;
			error_SUBSHELL:mrsh_command_destroy(&d->command);
			return NULL;
		}
		case MRSH_IF_CLAUSE: {
			const struct mrsh_if_clause *s=(const void*)c;
			struct mrsh_if_clause *d=calloc(1,sizeof *d);
			if(!d)return NULL;
			*d=*s;
			d->else_part=NULL;
			d->condition=(struct mrsh_array) {
				0
			};
			d->body=(struct mrsh_array) {
				0
			};
			d->command.redirects=NULL;
			if(s->else_part && !(d->else_part=mrsh_command_copy(s->else_part)))goto error_IF_CLAUSE;
			if(!copy_lists(&d->condition,&s->condition))goto error_IF_CLAUSE;
			if(!copy_lists(&d->body,&s->body))goto error_IF_CLAUSE;
			result=&d->command;
			break;
			error_IF_CLAUSE:mrsh_command_destroy(&d->command);
			return NULL;
		}
		case MRSH_FOR_CLAUSE: {
			const struct mrsh_for_clause *s=(const void*)c;
			struct mrsh_for_clause *d=calloc(1,sizeof *d);
			if(!d)return NULL;
			*d=*s;
			d->name=NULL;
			d->word_list=(struct mrsh_array) {
				0
			};
			d->body=(struct mrsh_array) {
				0
			};
			d->command.redirects=NULL;
			if(s->name && !(d->name=strdup(s->name)))goto error_FOR_CLAUSE;
			if(!copy_words(&d->word_list,&s->word_list))goto error_FOR_CLAUSE;
			if(!copy_lists(&d->body,&s->body))goto error_FOR_CLAUSE;
			result=&d->command;
			break;
			error_FOR_CLAUSE:mrsh_command_destroy(&d->command);
			return NULL;
		}
		case MRSH_LOOP_CLAUSE: {
			const struct mrsh_loop_clause *s=(const void*)c;
			struct mrsh_loop_clause *d=calloc(1,sizeof *d);
			if(!d)return NULL;
			*d=*s;
			d->condition=(struct mrsh_array) {
				0
			};
			d->body=(struct mrsh_array) {
				0
			};
			d->command.redirects=NULL;
			if(!copy_lists(&d->condition,&s->condition))goto error_LOOP_CLAUSE;
			if(!copy_lists(&d->body,&s->body))goto error_LOOP_CLAUSE;
			result=&d->command;
			break;
			error_LOOP_CLAUSE:mrsh_command_destroy(&d->command);
			return NULL;
		}
		case MRSH_CASE_CLAUSE: {
			const struct mrsh_case_clause *s=(const void*)c;
			struct mrsh_case_clause *d=calloc(1,sizeof *d);
			if(!d)return NULL;
			*d=*s;
			d->word=NULL;
			d->items=(struct mrsh_array) {
				0
			};
			d->command.redirects=NULL;
			if(s->word && !(d->word=mrsh_word_copy(s->word)))goto error_CASE_CLAUSE;
			if(!copy_cases(&d->items,&s->items))goto error_CASE_CLAUSE;
			result=&d->command;
			break;
			error_CASE_CLAUSE:mrsh_command_destroy(&d->command);
			return NULL;
		}
		case MRSH_FUNCTION_DEFINITION: {
			const struct mrsh_function_definition *s=(const void*)c;
			struct mrsh_function_definition *d=calloc(1,sizeof *d);
			if(!d)return NULL;
			*d=*s;
			d->name=NULL;
			d->body=NULL;
			d->io_redirects=(struct mrsh_array) {
				0
			};
			d->command.redirects=NULL;
			if(s->name && !(d->name=strdup(s->name)))goto error_FUNCTION_DEFINITION;
			if(s->body && !(d->body=mrsh_command_copy(s->body)))goto error_FUNCTION_DEFINITION;
			if(!copy_redirects(&d->io_redirects,&s->io_redirects))goto error_FUNCTION_DEFINITION;
			result=&d->command;
			break;
			error_FUNCTION_DEFINITION:mrsh_command_destroy(&d->command);
			return NULL;
		}
	}
	if(result&&c->redirects) {
		result->redirects=calloc(1,sizeof *result->redirects);
		if(!result->redirects||!copy_redirects(result->redirects,c->redirects)) {
			mrsh_command_destroy(result);
			return NULL;
		}
	}
	return result;
}

void word_array_finish(struct mrsh_array *a) { for(size_t i=0;i<a->len;i++)mrsh_word_destroy(a->data[i]); mrsh_array_finish(a); }
void redirect_array_finish(struct mrsh_array *a) { for(size_t i=0;i<a->len;i++)mrsh_io_redirect_destroy(a->data[i]); mrsh_array_finish(a); }
void assignment_array_finish(struct mrsh_array *a) { for(size_t i=0;i<a->len;i++)mrsh_assignment_destroy(a->data[i]); mrsh_array_finish(a); }
