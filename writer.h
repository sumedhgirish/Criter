
// Header Part
#ifndef WRITER_H_
#define WRITER_H_
#include <cassert>
#include <cstdarg>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#endif

typedef enum
{
	OK,
	INFO,
	DEBUG,
	WARN,
	ERROR
} LogLevel;

typedef enum
{
	UINT64,
	INT,
	STR,
	FLOAT,
	DOUBLE,
	CHAR,
	GENERICPTR
} DataType;

typedef union
{
	uint64_t t_uint64;
	int t_int;
	const char* t_str;
	float t_float;
	double t_double;
	char t_char;
	void* t_genericptr;
} Any;

typedef struct Logs Logs;
struct Logs
{
	LogLevel priority;
	const char* data;
	Logs* next;
};

typedef struct
{
	const DataType type;
	Any data;
	Logs* tail;
} DataWithLogs;

DataWithLogs Flatten(DataWithLogs* inp, DataWithLogs (*function)(Any, va_list), ...);

void PrependLogs(Logs* from_tail, Logs* to_tail);
void ConcatLogs(Logs* main, Logs* secondary);
void AddLog(Logs* main, LogLevel priority, const char* message);
// Move to documentation: All logs having a priority < specified arg are removed
void FilterLogs(Logs** main, LogLevel priority);
void ClearLogs(Logs** main);

// TODO: Add functionality to display the logs of some data in various ways.

/////////////////////////////////////////////////////////
#define WRITER_IMPLEMENTATION
// Implementation
#ifdef WRITER_IMPLEMENTATION

void PrependLogs(Logs* from_tail, Logs* to_tail)
{
	if(from_tail == NULL)
		return;
	Logs* tmp = from_tail;
	if(to_tail != NULL)
	{
		to_tail = from_tail;
		while(tmp->next != NULL)
			tmp = tmp->next;
		tmp->next = to_tail;
	}
	to_tail = from_tail;
}

void ConcatLogs(Logs* main, Logs* secondary)
{
	if(main == NULL)
		return;
	Logs* tmp;
	tmp = main;
	while(tmp->next != NULL)
		tmp = tmp->next;

	tmp->next = secondary;
}

void AddLog(Logs* main, LogLevel priority, const char* message)
{
	assert(message != NULL);
	assert(main != NULL);

	Logs* newLogs = (Logs*)malloc(sizeof(Logs));
	assert(newLogs != NULL);

	newLogs->priority = priority;
	newLogs->data = message;
	newLogs->next = NULL;

	ConcatLogs(main, newLogs);
}

void FilterLogs(Logs** main, LogLevel priority)
{
	assert(main != NULL);
	if(*main == NULL)
		return;

	Logs* tmp = *main;
	while(tmp != NULL && tmp->priority < priority)
	{
		*main = tmp->next;
		free(tmp);
		tmp = *main;
		continue;
	}

	Logs* tmp_next = NULL;
	while(tmp->next != NULL)
	{
		if(tmp->next->priority < priority)
		{
			tmp_next = tmp->next->next;
			free(tmp->next);
			tmp->next = tmp_next;
		}
		else
		{
			tmp = tmp->next;
		}
	}
}

void ClearLogs(Logs** main)
{
	assert(main != NULL);
	if(*main == NULL)
		return;

	Logs* tmp = *main;
	while(tmp->next)
	{
		*main = tmp->next;
		free(tmp);
		tmp = *main;
	}
	free(tmp);
	*main = NULL;
}

DataWithLogs Flatten(DataWithLogs* inp, DataWithLogs (*function)(Any, va_list), ...)
{
	assert(inp != NULL);

	va_list vp;
	va_start(vp, function);
	DataWithLogs result = function(inp->data, vp);
	va_end(vp);

	PrependLogs(inp->tail, result.tail);
	return result;
}

#endif
