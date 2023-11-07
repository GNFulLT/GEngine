#ifndef FIRST_H
#define FIRST_H

#include "gobject/gobject_defs.h"

struct First
{
public:
	GMETHOD()
	int b(int a);

	GMETHOD()
	char sum(int a);

	GMETHOD()
	int div(int a);
	
	GPROPERTY()
	char number;
};




class Second
{
public:
	GMETHOD()
	int b(int a);

	GMETHOD()
	int sum(int a);

	GMETHOD()
	int div(int a);

	GPROPERTY()
	int number;

	GPROPERTY()
	char s;
	
	GPROPERTY()
	First a;
};


#endif // FIRST_H