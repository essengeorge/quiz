#ifndef QUIZZES_DATABASE_H
#define QUIZZES_DATABASE_H

/* by default 0 is success, -1 is internal error */
/* specific codes for this package are in range [10; 19] */

#define DB_CODE_NULL_ARGUMENT 10
int is_quiz_name_taken(const char *name);

#define DB_CODE_ALREADY_EXISTS 11
#define DB_CODE_BAD_REQUEST 12
int create_quiz(const char *name);

int delete_quiz(const char *name);

/* NOTE: doesn't allow '\n' in text */
int
add_question(const char *quiz_name, const char *text, int correct_answer,
	int answc, const char **answv);

#endif
