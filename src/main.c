#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "database.h"
#include "gameengine.h"


#define BUF_SIZE 256


int main(int argc, char **argv)
{
	int i, j;
	int code;
	int players;
	int *scores;
	const char *method;
	const char *quiz_name;
	char text[BUF_SIZE];
	char buf[BUF_SIZE];
	int answc;
	int correct_answer;
	char **answv;
	if (argc != 3) {
		fprintf(stderr, "Usage: %s [method] [quiz name]\n", argv[0]);
		fprintf(stderr, "Methods: create, add, delete, run\n");
		return 1;
	}
	method = argv[1];
	quiz_name = argv[2];
	if (strcmp(method, "create") == 0) {
		code = create_quiz(quiz_name);
		switch (code) {
		case 0:
			printf("Successfully created quiz!\n");
			break;
		case DB_CODE_ALREADY_EXISTS:
			printf("Error: quiz with such name already exists\n");
			break;
		case DB_CODE_BAD_REQUEST:
			printf("Error: invalid input\n");
			break;
		default:
			printf("An error occured\n");
			break;
		}
		return (code == 0) ? 0 : 2;
	}
	if (strcmp(method, "delete") == 0) {
		code = delete_quiz(quiz_name);
		switch (code) {
		case 0:
			printf("Successfully deleted quiz!\n");
			break;
		case DB_CODE_BAD_REQUEST:
			printf("Error: invalid input\n");
			break;
		default:
			printf("An error occured\n");
			break;
		}
		return (code == 0) ? 0 : 3;
	}
	if (strcmp(method, "add") == 0) {
		printf("Input the question text:\n");
		if (!fgets(text, sizeof(text), stdin)) {
			return 8;
		}
		text[strcspn(text, "\n")] = '\0';
		printf("Input the answers count:\n");
		if (!fgets(buf, sizeof(buf), stdin)) {
			return 8;
		}
		buf[strcspn(buf, "\n")] = '\0';
		answc = atoi(buf);
		printf("Input the correct answer number (1-indexed):\n");
		if (!fgets(buf, sizeof(buf), stdin)) {
			return 8;
		}
		buf[strcspn(buf, "\n")] = '\0';
		correct_answer = atoi(buf);
		answv = malloc(sizeof(char *) * answc);
		if (!answv) {
			return 6;
		}
		printf("Now input the answers\n");
		for (i = 0; i < answc; i++) {
			answv[i] = malloc(BUF_SIZE);
			if (!answv[i]) {
				for (j = 0; j < i; j++) {
					free(answv[j]);
				}
				free(answv);
				return 7;
			}
			printf("Input the answer number %d:\n", i + 1);
			if (!fgets(answv[i], BUF_SIZE, stdin)) {
				for (j = 0; j <= i; j++) {
					free(answv[j]);
				}
				free(answv);
				return 7;
			}
			answv[i][strcspn(answv[i], "\n")] = '\0';
		}
		code = add_question(quiz_name, text, correct_answer, answc,
			(const char **)answv);
		switch (code) {
		case 0:
			printf("Successfully added question!\n");
			break;
		case DB_CODE_BAD_REQUEST:
			printf("Error: invalid input");
			break;
		default:
			printf("An error occured\n");
			break;
		}
		for (i = 0; i < answc; i++) {
			free(answv[i]);
		}
		free(answv);
		return (code == 0) ? 0 : 4;
	}
	if (strcmp(method, "run") == 0) {
		printf("Enter number of players:\n");
		if (!fgets(buf, sizeof(buf), stdin)) {
			return 8;
		}
		players = atoi(buf);
		if (players == 0) {
			fprintf(stderr, "Zero players? Heh\n");
			return 0;
		}
		if (players < 1) {
			fprintf(stderr, "Invalid number of players\n");
			return 8;
		}
		scores = calloc(players, sizeof(int));
		if (!scores) {
			return 9;
		}
		code = run_quiz(quiz_name, players, scores);
		switch (code) {
		default:
			break;
		}
		free(scores);
		return (code == 0) ? 0 : 5;
	}
	return 0;
}
