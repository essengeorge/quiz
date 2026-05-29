#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#include "database.h"

#define BUF_SIZE 256

static const char quizzes_file[] = "quizzes_list.txt";

static int ispunctuation(char c)
{
	switch (c) {
	case '.':
	case ',':
	case ';':
	case ':':
	case '\'':
	case '"':
	case '!':
	case '?':
	case '(':
	case ')':
	case '-':
		return 1;
	default:
		return 0;
	}
}

static int is_valid_quiz_char(char c)
{
	return isalpha((unsigned char)c) ||
		isdigit((unsigned char)c) ||
		ispunctuation((unsigned char)c) ||
		c == ' ';
}

static int is_valid_quiz_name(const char *name)
{
	char c;
	if (!name) {
		return 0;
	}
	while ((c = *name++)) {
		if (!is_valid_quiz_char(c)) {
			return 0;
		}
	}
	return 1;
}

static int (* const is_valid_question_text)(const char*) = is_valid_quiz_name;
static int (* const is_valid_answer_text)(const char*) = is_valid_quiz_name;

int is_quiz_name_taken(const char *name)
{
	int c;
	char buf[BUF_SIZE] = {0};
	int buf_idx = 0;
	FILE *f;
	if (!name) {
		return DB_CODE_NULL_ARGUMENT;
	}
	if (!is_valid_quiz_name(name)) {
		return DB_CODE_BAD_REQUEST;
	}
	f = fopen(quizzes_file, "r");
	if (!f) {
		if (errno == ENOENT) {
			return 0;
		}
		return -1;
	}
	while ((c = fgetc(f)) != EOF) {
		if ((char)c == '\n') {
			buf[buf_idx] = '\0';
			if (strcmp(buf, name) == 0) {
				fclose(f);
				return 1;
			}
			buf_idx = 0;
		} else {
			if (buf_idx + 1 >= BUF_SIZE) {
				buf_idx = 0;
				for (c = fgetc(f);
					c != EOF && c != '\n'; c = fgetc(f)) {}
				continue;
			}
			buf[buf_idx] = c;
			buf_idx++;
		}
	}
	if (buf_idx != 0) {
		buf[buf_idx] = '\0';
		if (strcmp(buf, name) == 0) {
			fclose(f);
			return 1;
		}
	}
	fclose(f);
	return 0;
}

int create_quiz(const char *name)
{
	int code;
	FILE *f_list;
	FILE *f_quiz;
	if (!name) {
		return DB_CODE_NULL_ARGUMENT;
	}
	code = is_quiz_name_taken(name);
	if (code != 0) {
		if (code == 1) {
			return DB_CODE_ALREADY_EXISTS;
		}
		if (code == DB_CODE_BAD_REQUEST) {
			return code;
		}
		return -1;
	}
	f_list = fopen(quizzes_file, "a");
	if (!f_list) {
		return -1;
	}
	f_quiz = fopen(name, "w");
	if (!f_quiz) {
		fclose(f_list);
		return -1;
	}
	fprintf(f_list, "%s\n", name);
	fprintf(f_quiz, "QUIZ\n%s\n", name);
	fclose(f_list);
	fclose(f_quiz);
	return 0;
}

int delete_quiz(const char *name)
{
	char buf[BUF_SIZE] = {0};
	size_t tmp_name_len;
	char *tmp_file_name;
	FILE *f;
	FILE *f_tmp;
	if (!name) {
		return DB_CODE_NULL_ARGUMENT;
	}
	if (!is_valid_quiz_name(name)) {
		return DB_CODE_BAD_REQUEST;
	}
	f = fopen(quizzes_file, "r");
	if (!f) {
		return -1;
	}
	tmp_name_len = strlen(quizzes_file) + 4;
	tmp_file_name = malloc(tmp_name_len + 1);
	if (!tmp_file_name) {
		fclose(f);
		return -1;
	}
	strcpy(tmp_file_name, quizzes_file);
	strcat(tmp_file_name, ".tmp");
	f_tmp = fopen(tmp_file_name, "w");
	if (!f_tmp) {
		free(tmp_file_name);
		fclose(f);
		return -1;
	}
	while (fgets(buf, BUF_SIZE, f)) {
		buf[strcspn(buf, "\n")] = '\0';
		if (strcmp(buf, name) == 0) {
			continue;
		}
		fprintf(f_tmp, "%s\n", buf);
	}
	fclose(f_tmp);
	fclose(f);
	remove(name);
	remove(quizzes_file);
	rename(tmp_file_name, quizzes_file);
	free(tmp_file_name);
	return 0;
}

int
add_question(const char *quiz_name, const char *text, int correct_answer,
	int answc, const char **answv)
{
	int i;
	FILE *f;
	if (!quiz_name || !text || !answv) {
		return DB_CODE_NULL_ARGUMENT;
	}
	if (
		is_quiz_name_taken(quiz_name) != 1 ||
		!is_valid_question_text(text) ||
		(correct_answer < 1) ||
		(correct_answer > answc)
	) {
		return DB_CODE_BAD_REQUEST;
	}
	for (i = 0; i < answc; i++) {
		if (!answv[i]) {
			return DB_CODE_NULL_ARGUMENT;
		}
		if (!is_valid_answer_text(answv[i])) {
			return DB_CODE_BAD_REQUEST;
		}
	}
	f = fopen(quiz_name, "a");
	if (!f) {
		return -1;
	}
	fprintf(f,
	"QUESTION\n"
	"%s\n"
	"ANSWERS COUNT\n"
	"%d\n"
	"CORRECT ANSWER\n"
	"%d\n",
	text, answc, correct_answer);
	fprintf(f,
	"ANSWERS\n");
	for (i = 0; i < answc; i++) {
		fprintf(f,
	"%d\n"
	"%s\n", i + 1, answv[i]);
	}
	fclose(f);
	return 0;
}
