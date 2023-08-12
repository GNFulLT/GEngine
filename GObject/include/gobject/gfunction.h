#ifndef GMETHOD_H
#define GMETHOD_H


#include "gobject/GEngine_EXPORT.h"
#include "public/typedefs.h"
#include <string_view>
#include "gobject/gfunction_execution_error.h"
#include <vector>
#include <expected>

class GVariant;
class GFunctionWrapper;

class GOBJECT_API GFunction
{
public:
	GFunction();
	GFunction(GFunctionWrapper* wrapper);
	
	
	bool is_valid() const _NO_EXCEPT_;
		
	std::string_view get_name() const _NO_EXCEPT_;

	bool is_static() const _NO_EXCEPT_;

	std::expected<GVariant, GFUNCTION_EXECUTION_ERROR> invoke(GVariant instance);
	std::expected<GVariant, GFUNCTION_EXECUTION_ERROR> invoke(GVariant instance, GVariant param1);
	std::expected<GVariant, GFUNCTION_EXECUTION_ERROR> invoke(GVariant instance, GVariant param1, GVariant param2);
	std::expected<GVariant, GFUNCTION_EXECUTION_ERROR> invoke(GVariant instance, GVariant param1, GVariant param2, GVariant param3);
	std::expected<GVariant, GFUNCTION_EXECUTION_ERROR> invoke(GVariant instance, std::vector<GVariant>& variadicParams);
private:
	GFunctionWrapper* m_wrapper;
};


#endif // GMETHOD_H