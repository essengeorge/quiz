#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "database.h"
#include "gameengine.h"


#define BUF_SIZE 256
#define COUNT_STEPS 5


struct question {
	char text[BUF_SIZE];
	int answc;
	char **answv;
	int correct_answer;
	int ok;
};


/* that function thinks you have already consumed the "QUESTION" line */
static struct question parse_question(FILE *f)
{
	int j;
	int i;
	char buf[BUF_SIZE];
	struct question q;
	struct question bad;
	bad.ok = 0;
	if (!fgets(q.text, sizeof(q.text), f)) {
		return bad;
	}
	q.text[strcspn(q.text, "\n")] = '\0';
	if (!fgets(buf, sizeof(buf), f)) {
		return bad;
	}
	if (!fgets(buf, sizeof(buf), f)) {
		return bad;
	}
	buf[strcspn(buf, "\n")] = '\0';
	q.answc = atoi(buf);
	if (!fgets(buf, sizeof(buf), f)) {
		return bad;
	}
	if (!fgets(buf, sizeof(buf), f)) {
		return bad;
	}
	q.correct_answer = atoi(buf);
	if (!fgets(buf, sizeof(buf), f)) {
		return bad;
	}
	q.answv = malloc(sizeof(char *) * q.answc);
	for (i = 0; i < q.answc; i++) {
		q.answv[i] = malloc(BUF_SIZE);
		if (!q.answv[i]) {
			for (j = 0; j < i; j++) {
				free(q.answv[j]);
			}
			free(q.answv);
			return bad;
		}
		if (!fgets(buf, sizeof(buf), f)) {
			return bad;
		}
		if (!fgets(q.answv[i], BUF_SIZE, f)) {
			for (j = 0; j < i; j++) {
				free(q.answv[j]);
			}
			free(q.answv);
			return bad;
		}
		q.answv[i][strcspn(q.answv[i], "\n")] = '\0';
	}
	q.ok = 1;
	return q;
}

static void clear_question(struct question *q)
{
	int i;
	if (!q) {
		return;
	}
	for (i = 0; i < q->answc; i++) {
		free(q->answv[i]);
	}
	free(q->answv);
	q->answv = NULL;
}

static void count_back(int steps)
{
	int i;
	if (steps < 1) {
		return;
	}
	for (i = steps; i >= 1; i--) {
		printf("%d...\n", i);
		sleep(1);
	}
}

static void run_question(struct question q, int players, int *scores)
{
	int i;
	int j;
	int answer;
	char buf[BUF_SIZE];
	if (!scores) {
		return;
	}
	if (players < 1) {
		return;
	}
	for (i = 0; i < players; i++) {
		printf("\033[2J\033[H");
		count_back(COUNT_STEPS);
		printf("\033[2J\033[H");
		printf("%s\n", q.text);
		for (j = 0; j < q.answc; j++) {
			printf("%d) %s\n", j + 1, q.answv[j]);
		}
		while (1) {
			if (!fgets(buf, sizeof(buf), stdin)) {
				return;
			}
			buf[strcspn(buf, "\n")] = '\0';
			answer = atoi(buf);
			if (answer >= 1 && answer <= q.answc) {
				break;
			}
		}
		if (answer == q.correct_answer) {
			printf("Correct!\n");
			scores[i]++;
		} else {
			printf("Incorrect!\n");
		}
		sleep(3);
		if (players > 1) {
			printf("\033[2J\033[H");
			printf("Pass the device to the next player\n");
			printf("Press enter when you will ready\n");
			if (!fgets(buf, sizeof(buf), stdin)) {
				continue;
			}
		}
	}
}

int run_quiz(const char *name, int players, int *scores)
{
	int i;
	struct question q;
	char buf[BUF_SIZE];
	FILE *f;
	if (!name || !scores) {
		return GE_CODE_NULL_ARGUMENT;
	}
	if (is_quiz_name_taken(name) != 1 || players < 1) {
		return GE_CODE_BAD_REQUEST;
	}
	f = fopen(name, "r");
	if (!f) {
		return -1;
	}
	if (!fgets(buf, sizeof(buf), f)) {
		fclose(f);
		return -1;
	}
	buf[strcspn(buf, "\n")] = '\0';
	if (strcmp(buf, "QUIZ") != 0) {
		fclose(f);
		return -1;
	}
	if (!fgets(buf, sizeof(buf), f)) {
		fclose(f);
		return -1;
	}
	buf[strcspn(buf, "\n")] = '\0';
	printf("Welcome to the quiz \"%s\"\n", buf);
	if (!fgets(buf, sizeof(buf), f)) {
		fclose(f);
		return -1;
	}
	while (q = parse_question(f), q.ok) {
		run_question(q, players, scores);
		clear_question(&q);
		if (!fgets(buf, sizeof(buf), f)) {
			break;
		}
	}
	printf("SCORES:\n");
	for (i = 0; i < players; i++) {
		printf("%d: %d\n", i + 1, scores[i]);
	}
	fclose(f);
	return 0;
}
