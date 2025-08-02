
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
void AddLog(Logs* main, LogLevel priority, const char* message);

// Move to documentation: All logs having a priority < specified arg are removed
// Port this to work with DataWithLogs instead of the Logs themselves
void FilterLogs(Logs** main, LogLevel priority);
void ClearLogs(Logs** main);

// TODO: Add functionality to display the logs of some data in various ways.

/////////////////////////////////////////////////////////
#define WRITER_IMPLEMENTATION
// Implementation
#ifdef WRITER_IMPLEMENTATION

static Logs* __new(LogLevel level, const char* message)
{
	Logs* out = (Logs*)malloc(sizeof(Logs));
	assert(out != NULL);
	out->priority = level;
	out->data = message;
	out->next = NULL;
	return out;
}

static void __append(Logs* from_node, Logs* to_node)
{
	if(from_node == NULL)
		return;

	Logs* tmp = to_node;
	while(tmp != NULL)
		tmp = tmp->next;

	Logs* ref = from_node;
	while(ref != NULL)
	{
		tmp = __new(ref->priority, ref->data);
		ref = ref->next;
		tmp = tmp->next;
	}
}

static void __prepend(Logs* from_node, Logs* to_node)
{
	Logs* tmp = NULL;
	__append(from_node, tmp);
	Logs* tmp_tail = tmp;
	while(tmp != NULL)
		tmp = tmp->next;
	tmp = to_node;
	to_node = tmp_tail;
}

void __remove_partial(Logs** from_node, LogLevel threshold)
{
	assert(from_node != NULL);
	if(*from_node == NULL)
		return;

	Logs* tmp = *from_node;
	while(tmp != NULL && tmp->priority < threshold)
	{
		*from_node = tmp->next;
		free(tmp);
		tmp = *from_node;
	}
	if(*from_node == NULL)
		return;

	Logs* tmp_next = NULL;
	while(tmp->next != NULL)
	{
		if(tmp->next->priority < threshold)
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

static void __remove_all(Logs** from_node)
{
	assert(from_node != NULL);
	if(*from_node == NULL)
		return;
	Logs* tmp = (*from_node)->next;
	while(tmp != NULL)
	{
		tmp = tmp->next;
		free((*from_node)->next);
		(*from_node)->next = tmp;
	}
	free(*from_node);
	*from_node = NULL;
}

DataWithLogs Flatten(DataWithLogs* inp, DataWithLogs (*function)(Any, va_list), ...)
{
	assert(inp != NULL);

	va_list vp;
	va_start(vp, function);
	DataWithLogs result = function(inp->data, vp);
	va_end(vp);

	__prepend(inp->tail, result.tail);
	return result;
}

#endif
