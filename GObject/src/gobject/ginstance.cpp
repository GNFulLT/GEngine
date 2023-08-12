#include "gobject/ginstance.h"


bool GInstance::operator=(const GInstance& other)
{
	return m_rawPointer == other.m_rawPointer;
}