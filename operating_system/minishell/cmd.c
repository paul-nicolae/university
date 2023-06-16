// SPDX-License-Identifier: BSD-3-Clause

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <fcntl.h>
#include <unistd.h>

#include "cmd.h"
#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define READ 0
#define WRITE 1

static bool shell_cd(word_t *dir)
{
	int rc;

	// chdir() changes the current working directory of the
	// calling process to the directory specified in `path`.

	if (dir == NULL) {
		rc = chdir(getenv("HOME"));
		if (rc == 0)
			return 0;
		return 1;
	}

	char *path = get_word(dir);

	rc = chdir(path);

	if (rc == 0)
		return 0;

	return 1;
}

static int shell_exit(void)
{
	exit(0);
}

static int parse_simple(simple_command_t *s, int level, command_t *father)
{
	// sanity check
	if (s == NULL)
		return SHELL_EXIT;

	if (strcmp(s->verb->string, "false") == 0)
		return 1;
	if (strcmp(s->verb->string, "true") == 0)
		return 0;

	int size, fd, status, rc, fds[3];

	// redirect standard file descriptors
	fds[0] = dup(STDIN_FILENO);
	fds[1] = dup(STDOUT_FILENO);
	fds[2] = dup(STDERR_FILENO);

	// open all stardard file descriptors
	if (s->in != NULL) {
		fd = open(get_word(s->in), O_RDONLY);
		DIE(fd < 0, "open");
		dup2(fd, STDIN_FILENO);
		close(fd);
	}

	if (s->out != NULL) {
		if (s->io_flags & IO_OUT_APPEND)
			fd = open(get_word(s->out), O_WRONLY | O_CREAT | O_APPEND, 0644);
		else
			fd = open(get_word(s->out), O_WRONLY | O_CREAT | O_TRUNC, 0644);
		DIE(fd < 0, "open");
		dup2(fd, STDOUT_FILENO);
		close(fd);
	}

	if (s->err != NULL) {
		if (s->out != NULL && strcmp(s->out->string, s->err->string) == 0) {
			// if out and err are redirected to the same file
			dup2(STDOUT_FILENO, STDERR_FILENO);
		} else {
			if (s->io_flags & IO_ERR_APPEND)
				fd = open(get_word(s->err), O_WRONLY | O_CREAT | O_APPEND, 0644);
			else
				fd = open(get_word(s->err), O_WRONLY | O_CREAT | O_TRUNC, 0644);
			DIE(fd < 0, "open");
			dup2(fd, STDERR_FILENO);
			close(fd);
		}
	}

	/* internal commands */

	// internal exit/quit command
	if (!strcmp(s->verb->string, "exit") || !strcmp(s->verb->string, "quit"))
		shell_exit();

	// internal change-directory command
	if (!strcmp(s->verb->string, "cd")) {
		rc = shell_cd(s->params);

		// restore standard file descriptors
		dup2(fds[0], STDIN_FILENO);
		dup2(fds[1], STDOUT_FILENO);
		dup2(fds[2], STDERR_FILENO);

		return rc;
	}

	/* environment variable assignment */

	// if command has an equal sign, it's a variable assignment
	if (s->verb->next_part != NULL) {
		int rc;

		const char *name = s->verb->string;
		const char *value = get_word(s->verb->next_part->next_part);

		rc = setenv(name, value, 1);

		if (rc == 0)
			return EXIT_SUCCESS;
		return EXIT_FAILURE;
	}

	/* external commands */

	pid_t pid;

	pid = fork();
	switch (pid) {
	case -1: // fork failed, cleaning up
		DIE(pid, "fork");
		return EXIT_FAILURE;

	case 0: // child process

		// execute external command
		execvp(s->verb->string, get_argv(s, &size));

		// if command is unknown, execvp() will not return
		printf("Execution failed for '%s'\n", s->verb->string);
		exit(-1);

	default: // parent process

		// wait for child process to finish
		rc = waitpid(pid, &status, 0);
		DIE(rc < 0, "waitpid");

		break;
	}

	// restore standard file descriptors
	dup2(fds[0], STDIN_FILENO);
	dup2(fds[1], STDOUT_FILENO);
	dup2(fds[2], STDERR_FILENO);

	// return the exit status of the child process
	return WEXITSTATUS(status);
}

static bool run_in_parallel(command_t *cmd1, command_t *cmd2, int level, command_t *father)
{
	pid_t pid, ret_pid;
	int status, rc;

	pid = fork();
	switch (pid) {
	case -1: // fork failed, cleaning up
		DIE(pid, "fork");
		return EXIT_FAILURE;

	case 0: // child process

		// execute cmd1
		rc = parse_command(cmd1, level + 1, father);
		exit(rc);

	default: // parent process

		// execute cmd2
		rc = parse_command(cmd2, level + 1, father);

		// wait for child to finish
		ret_pid = waitpid(pid, &status, 0);
		DIE(ret_pid < 0, "waitpid");

		break;
	}

	// return exit status of child process
	return WEXITSTATUS(status);
}

static bool run_on_pipe(command_t *cmd1, command_t *cmd2, int level, command_t *father)
{
	pid_t pid, ret_pid;
	int pipefd[2], rc, status, fdin;

	// save stdin
	fdin = dup(STDIN_FILENO);

	rc = pipe(pipefd);
	DIE(rc < 0, "pipe");

	pid = fork();
	switch (pid) {
	case -1: // fork failed, cleaning up
		DIE(pid, "fork");
		return EXIT_FAILURE;

	case 0: // child process

		// close unused pipe head
		rc = close(pipefd[READ]);
		DIE(rc < 0, "close");

		// redirect stdout to write head of the pipe
		rc = dup2(pipefd[WRITE], STDOUT_FILENO);
		DIE(rc < 0, "dup2");

		// executes cmd1
		rc = parse_command(cmd1, level + 1, father);
		exit(rc);

	default: // parent process

		// close unused pipe head
		rc = close(pipefd[WRITE]);
		DIE(rc < 0, "close");

		// redirect stdin to read head of the pipe
		rc = dup2(pipefd[READ], STDIN_FILENO);
		DIE(rc < 0, "dup2");

		// executes cmd2
		rc = parse_command(cmd2, level + 1, father);

		// wait for child to finish
		ret_pid = waitpid(pid, &status, 0);
		DIE(ret_pid < 0, "waitpid");

		break;
	}

	// restore stdin
	dup2(fdin, STDIN_FILENO);

	// return the exit status of the second command
	return rc;
}

int parse_command(command_t *c, int level, command_t *father)
{
	// sanity checks
	if (c == NULL)
		return SHELL_EXIT;

	// save exist status of the command in rc
	int rc;

	if (c->op == OP_NONE) {

		// execute a simple command
		rc = parse_simple(c->scmd, level, father);

		// return the exit status of the command
		return rc;
	}

	switch (c->op) {
	case OP_SEQUENTIAL:

		// execute the commands one after the other
		rc = parse_command(c->cmd1, level + 1, c);
		if (rc == SHELL_EXIT)
			return SHELL_EXIT;

		rc = parse_command(c->cmd2, level + 1, c);

		break;

	case OP_PARALLEL:

		// execute the commands in parallel
		rc = run_in_parallel(c->cmd1, c->cmd2, level + 1, c);

		break;

	case OP_CONDITIONAL_NZERO:

		// execute the second command only if the first one returns non zero
		rc = parse_command(c->cmd1, level + 1, c);

		if (rc != 0)
			rc = parse_command(c->cmd2, level + 1, c);

		break;

	case OP_CONDITIONAL_ZERO:

		// execute the second command only if the first one returns zero
		rc = parse_command(c->cmd1, level + 1, c);

		if (rc == 0)
			rc = parse_command(c->cmd2, level + 1, c);

		break;

	case OP_PIPE:

		// redirect the output of the first command to the input of the second.
		rc = run_on_pipe(c->cmd1, c->cmd2, level + 1, c);

		break;

	default:
		return SHELL_EXIT;
	}

	// return the exit status of the command
	return rc;
}
