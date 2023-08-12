#include "gobject/gfunction.h"
#include "gobject/gfunction_wrapper.h"

GFunction::GFunction()
{
	m_wrapper = nullptr;
}


GFunction::GFunction(GFunctionWrapper* wrapper)
{
	m_wrapper = wrapper;
}

std::expected<GVariant, GFUNCTION_EXECUTION_ERROR> GFunction::invoke(GVariant instance)
{
	if (!is_valid())
		return std::unexpected(GFUNCTION_EXECUTION_ERROR::GFUNCTION_EXECUTION_ERROR_INVALID_FUNCTION);
	return m_wrapper->invoke(instance);
}

std::expected<GVariant, GFUNCTION_EXECUTION_ERROR> GFunction::invoke(GVariant instance, GVariant param1)
{
	if (!is_valid())
		return std::unexpected(GFUNCTION_EXECUTION_ERROR::GFUNCTION_EXECUTION_ERROR_INVALID_FUNCTION);
	return m_wrapper->invoke(instance,param1);
}

std::expected<GVariant, GFUNCTION_EXECUTION_ERROR> GFunction::invoke(GVariant instance, GVariant param1, GVariant param2)
{
	if (!is_valid())
		return std::unexpected(GFUNCTION_EXECUTION_ERROR::GFUNCTION_EXECUTION_ERROR_INVALID_FUNCTION);
	return m_wrapper->invoke(instance,param1,param2);
}

std::expected<GVariant, GFUNCTION_EXECUTION_ERROR> GFunction::invoke(GVariant instance, GVariant param1, GVariant param2, GVariant param3)
{
	if (!is_valid())
		return std::unexpected(GFUNCTION_EXECUTION_ERROR::GFUNCTION_EXECUTION_ERROR_INVALID_FUNCTION);
	return m_wrapper->invoke(instance,param1,param2,param3);
}

std::expected<GVariant, GFUNCTION_EXECUTION_ERROR> GFunction::invoke(GVariant instance, std::vector<GVariant>& variadicParams)
{
	if (!is_valid())
		return std::unexpected(GFUNCTION_EXECUTION_ERROR::GFUNCTION_EXECUTION_ERROR_INVALID_FUNCTION);
	return m_wrapper->invoke(instance,variadicParams);
}


bool GFunction::is_valid() const _NO_EXCEPT_
{
	return m_wrapper != nullptr;
}

std::string_view GFunction::get_name() const _NO_EXCEPT_
{
	if (!is_valid())
		return "";
	return m_wrapper->get_name();
}