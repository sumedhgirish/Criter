#pragma once

// Header Part
#ifndef WRITER_H_
#	define WRITER_H_
#	include <assert.h>
#	include <stdarg.h>
#	include <stddef.h>
#	include <stdint.h>
#	include <stdio.h>
#	include <stdlib.h>
#	include <string.h>
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

#ifndef MAX_LOG_LENGTH
#	define MAX_LOG_LENGTH 256
#endif

typedef struct Logs Logs;
struct Logs
{
	LogLevel priority;
	char data[MAX_LOG_LENGTH];
	Logs* next;
};

typedef struct
{
	DataType type;
	Any data;
	Logs* tail;
} DataWithLogs;

DataWithLogs Wrap(DataType type, Any data);
void Log(DataWithLogs* to_ptr, LogLevel priority, const char* format, ...);
DataWithLogs Unit(DataWithLogs (*func)(Any), DataType type, DataWithLogs* inp_ptr);
void Display(FILE* stream, DataWithLogs* inp_ptr, LogLevel threshold);

/////////////////////////////////////////////////////////
#define WRITER_IMPLEMENTATION
// Implementation
#ifdef WRITER_IMPLEMENTATION

static Logs* __new(LogLevel level, const char* message)
{
	Logs* out = (Logs*)malloc(sizeof(Logs));
	assert(out != NULL);
	out->priority = level;
	strncpy(out->data, message, MAX_LOG_LENGTH);
	out->next = NULL;
	return out;
}

static void __append(Logs* from_node, Logs** to_node_ptr)
{
	if(from_node == NULL)
		return;
	assert(to_node_ptr != NULL);

	Logs* tmp = *to_node_ptr;
	if(tmp != NULL)
	{
		while(tmp->next != NULL)
			tmp = tmp->next;
		tmp->next = __new(from_node->priority, from_node->data);
		tmp = tmp->next;
	}
	else
		tmp = (*to_node_ptr) = __new(from_node->priority, from_node->data);

	Logs* ref = from_node->next;
	while(ref != NULL)
	{
		tmp->next = __new(ref->priority, ref->data);
		ref = ref->next;
		tmp = tmp->next;
	}
}

static void __append_immediate(Logs* from_node, Logs** to_node_ptr)
{
	if(from_node == NULL)
		return;
	assert(to_node_ptr != NULL);

	if(*to_node_ptr == NULL)
		*to_node_ptr = from_node;
	else
	{
		Logs* tmp = (*to_node_ptr);
		while(tmp->next != NULL)
			tmp = tmp->next;
		tmp->next = from_node;
	}
}

static void __prepend(Logs* from_node, Logs** to_node_ptr)
{
	assert(to_node_ptr != NULL);
	if(*to_node_ptr == NULL)
	{
		__append(from_node, to_node_ptr);
		return;
	}
	Logs *tmp = NULL, *swap = NULL;
	__append(from_node, &tmp);
	assert(tmp != NULL);
	swap = tmp;
	while(tmp->next != NULL)
		(tmp) = tmp->next;
	tmp->next = (*to_node_ptr);
	*to_node_ptr = swap;
}

static void __prepend_immediate(Logs* from_node, Logs** to_node_ptr)
{
	if(from_node == NULL)
		return;
	assert(to_node_ptr != NULL);

	if(*to_node_ptr == NULL)
		*to_node_ptr = from_node;
	else
	{
		Logs* tmp = *to_node_ptr;
		*to_node_ptr = from_node;
		while(from_node->next != NULL)
			from_node = from_node->next;
		from_node->next = tmp;
	}
}

static void __remove_partial(Logs** from_node_ptr, LogLevel threshold)
{
	assert(from_node_ptr != NULL);
	if(*from_node_ptr == NULL)
		return;

	Logs* tmp = *from_node_ptr;
	while(tmp != NULL && tmp->priority < threshold)
	{
		*from_node_ptr = tmp->next;
		free(tmp);
		tmp = *from_node_ptr;
	}
	if(*from_node_ptr == NULL)
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

static void __remove_all(Logs** from_node_ptr)
{
	assert(from_node_ptr != NULL);
	if(*from_node_ptr == NULL)
		return;
	Logs* tmp = (*from_node_ptr)->next;
	while(tmp != NULL)
	{
		tmp = tmp->next;
		free((*from_node_ptr)->next);
		(*from_node_ptr)->next = tmp;
	}
	free(*from_node_ptr);
	*from_node_ptr = NULL;
}

DataWithLogs Wrap(DataType type, Any data)
{
	return (DataWithLogs){type, data, NULL};
}

DataWithLogs Unit(DataWithLogs (*func)(Any), DataType type, DataWithLogs* inp_ptr)
{
	assert(type == inp_ptr->type);
	DataWithLogs output = func(inp_ptr->data);
	__prepend(inp_ptr->tail, &output.tail);
	return output;
}

void Log(DataWithLogs* to_ptr, LogLevel priority, const char* format, ...)
{
	va_list ap;
	va_start(ap, format);
	Logs* next = __new(priority, "");
	vsnprintf(next->data, MAX_LOG_LENGTH, format, ap);
	va_end(ap);
	__append_immediate(next, &(to_ptr->tail));
}

#	ifdef COLORIZE
#		define ANSI_RED "\x1b[31m"
#		define ANSI_GREEN "\x1b[32m"
#		define ANSI_YELLOW "\x1b[33m"
#		define ANSI_BLUE "\x1b[34m"
#		define ANSI_MAGENTA "\x1b[35m"
#		define ANSI_CYAN "\x1b[36m"
#		define ANSI_RESET "\x1b[0m"

static void __display_color(FILE* stream, Logs* from_ptr)
{
	const char *level, *color;
	switch(from_ptr->priority)
	{
	case ERROR:
		level = "ERROR";
		color = ANSI_RED;
		break;
	case WARN:
		level = "WARN ";
		color = ANSI_YELLOW;
		break;
	case DEBUG:
		level = "DEBUG";
		color = ANSI_BLUE;
		break;
	case INFO:
		level = "INFO ";
		color = ANSI_GREEN;
		break;
	case OK:
		level = "OKAY ";
		color = ANSI_RESET;
		break;
	}
	fprintf(stream, "[%s%s" ANSI_RESET "] %s\n", color, level, from_ptr->data);
	return;
}

#	else

static void __display(FILE* stream, Logs* from_ptr)
{
	const char* level;
	switch(from_ptr->priority)
	{
	case ERROR:
		level = "ERROR";
		break;
	case WARN:
		level = "WARN ";
		break;
	case DEBUG:
		level = "DEBUG";
		break;
	case INFO:
		level = "INFO ";
		break;
	case OK:
		level = "OKAY ";
		break;
	}
	fprintf(stream, "[%s] %s\n", level, from_ptr->data);
	return;
}
#	endif

void Display(FILE* stream, DataWithLogs* inp_ptr, LogLevel threshold)
{
	assert(stream != NULL);
	Logs* tmp = inp_ptr->tail;
	while(tmp)
	{
		if(tmp->priority >= threshold)
		{
#	ifdef COLORIZE
			__display_color(stream, tmp);
#	else
			__display(stream, tmp);
#	endif
		}
		tmp = tmp->next;
	}
}
#endif
