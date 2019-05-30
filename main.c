/*
 * main.cpp
 *
 *  Created on: May 27, 2019
 *      Author: 26sra
 */
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

extern int run();
char ** argvGlobal = NULL;

int cint_add(int a, int b) {
	int ret = a + b;
	if ((a > 0 && b > 0 && ret < 0) ||
		(a < 0 && b < 0 && ret > 0)) {
		printf("error: overflow on cint addition\n");
		exit(1);
	}
	return ret;
}

int cint_subtract(int a, int b) {
	int ret = a - b;
	if ((a < 0 && b > 0 && ret > 0) ||
		(a > 0 && b < 0 && ret < 0)) {
		printf("error: overflow on cint subtraction\n");
		exit(1);
	}
	return ret;
}

int cint_multiply(int a, int b) {
	int ret = a*b;
	if ((a > 0 && b > 0 && ret < 0) ||
		(a < 0 && b < 0 && ret < 0) ||
		(a < 0 && b > 0 && ret > 0) ||
		(a > 0 && b < 0 && ret > 0) ||
		(a==INT_MIN && b==-1) ||
		(a==-1 && b==INT_MIN)) {
		printf("error: overflow on cint multiplication\n");
		exit(1);
	}
	return ret;
}

int cint_divide(int a, int b) {
	if ((b==0) ||
		(a==INT_MIN && b==-1)) {
		printf("error: overflow on cint division\n");
		exit(1);
	}
	int ret = a/b;
	return ret;
}

int cint_negate(int a) {
	return cint_multiply(a, -1);
}

int arg(int i) {
	return atoi(argvGlobal[i+1]);
}

float argf(int i) {
	return atof(argvGlobal[i+1]);
}

int main(int argc, char ** argv) {
	argvGlobal = argv;
	return run();
}
