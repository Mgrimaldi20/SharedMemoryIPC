#pragma once

#include <stdbool.h>

#define KEY_ESC 27
#define MAX_CLIENTS 4

typedef enum
{
	CMD_NONE = 0,
	CMD_INIT,
	CMD_EXIT,
	CMD_KEYDOWN,
	CMD_KEYUP
} command_t;

typedef struct
{
	bool used;
	int key;
} clientstate_t;

typedef struct
{
	clientstate_t state[MAX_CLIENTS];
	int clientid;
	command_t cmd;
} serverstate_t;
