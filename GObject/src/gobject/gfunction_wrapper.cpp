#include "gobject/gfunction_wrapper.h"
#include "gobject/gtype.h"

GFunctionWrapper::GFunctionWrapper(std::string_view name, const GTypeInfo* declaring_type, const GTypeInfo* returnType, const std::vector<const GTypeInfo*>& parameters) _NO_EXCEPT_
{
	m_returnType = create_type(returnType);
	m_targetedType = create_type(declaring_type);
	m_methodName = name;
	m_parameters.resize(parameters.size());

	for (int i = 0; i < parameters.size(); i++)
	{
		m_parameters[i] = create_type(parameters[i]);
	}
}

std::string_view GFunctionWrapper::get_name() const _NO_EXCEPT_
{
	return m_methodName;
}
