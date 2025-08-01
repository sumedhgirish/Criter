
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
	GENERIC
} DataType;

typedef union
{
	uint64_t* int64ptr;
	int* intptr;
	const char** strptr;
	float* floatptr;
	double* doubleptr;
	char* charptr;
	void* genericptr;
} Anyptr;

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
	Anyptr data;
	Logs* tail;
} DataWithLogs;

DataWithLogs WrapWithLogs(DataType type, void* dataptr);
void Flatten(DataWithLogs* inp, DataWithLogs (*function)(Anyptr dataptr, va_list vlist), ...);
void FlattenNew(DataWithLogs* inp,
				DataWithLogs* output,
				DataWithLogs (*function)(Anyptr dataptr, va_list vlist),
				...);

void ConcatLogs(Logs* main, Logs* secondary);
void AddLog(Logs* main, LogLevel priority, const char* message);
// Move to documentation: All logs having a priority < specified arg are removed
void FilterLogs(Logs** main, LogLevel priority);
void ClearLogs(Logs** main);

/////////////////////////////////////////////////////////
// #define WRITER_IMPLEMENTATION // <--------------- REMOVE THIS LINE IN PRODUCTION
// Implementation
#ifdef WRITER_IMPLEMENTATION

DataWithLogs WrapWithLogs(DataType type, void* dataptr)
{
	DataWithLogs logs = {.type = type, .tail = NULL};

	switch(type)
	{
	case UINT64:
		logs.data.int64ptr = (uint64_t*)dataptr;
	case INT:
		logs.data.intptr = (int*)dataptr;
	case STR:
		logs.data.strptr = (const char**)dataptr;
	case FLOAT:
		logs.data.floatptr = (float*)dataptr;
	case DOUBLE:
		logs.data.doubleptr = (double*)dataptr;
	case CHAR:
		logs.data.charptr = (char*)dataptr;
	case GENERIC:
		logs.data.genericptr = dataptr;
	}

	return logs;
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

void Flatten(DataWithLogs* inp, DataWithLogs (*function)(Anyptr dataptr, va_list vlist), ...)
{
	assert(inp != NULL);

	va_list vp;
	va_start(vp, function);
	DataWithLogs result = function(inp->data, vp);
	va_end(vp);

	assert(inp->type == result.type);
	inp->data = result.data;
	ConcatLogs(inp->tail, result.tail);
}

void FlattenNew(DataWithLogs* inp,
				DataWithLogs* output,
				DataWithLogs (*function)(Anyptr dataptr, va_list vlist),
				...)
{
	assert(inp != NULL);

	va_list vp;
	va_start(vp, function);
	DataWithLogs result = function(inp->data, vp);
	va_end(vp);

	assert(output->type == result.type);
	output->data = result.data;
	ConcatLogs(output->tail, result.tail);
}

#endif
