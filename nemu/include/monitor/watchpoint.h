#ifndef __WATCHPOINT_H__
#define __WATCHPOINT_H__

#include "common.h"

typedef struct watchpoint {
	int NO;
	struct watchpoint *next;
	int old_val;
	int new_val;
	char str[32];
	int b;
	
	

	/* TODO: Add more members if necessary */


} WP;
WP* new_wp(char* string);
bool check_wp();
void delete_wp(int num);
void free_wp(WP *wp);
void info_wp();

#endif
