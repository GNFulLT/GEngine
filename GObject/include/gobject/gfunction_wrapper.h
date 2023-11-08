#ifndef GMETHOD_WRAPPER_H
#define GMETHOD_WRAPPER_H


#include "gobject/GEngine_EXPORT.h"
#include "public/typedefs.h"
#include <string_view>
#include <vector>
#include <string>
#include "gobject/gvariant.h"
#include <string_view>
#include <expected>
#include <tuple>
#include "gobject/gfunction_execution_error.h"
#include "gobject/gtype_utils.h"

struct GTypeInfo;

class GOBJECT_API GFunctionWrapper
{
public:
	GFunctionWrapper(std::string_view name, const GTypeInfo* declaring_type,const GTypeInfo* returnType,const std::vector<const GTypeInfo*>& parameters) _NO_EXCEPT_;

	virtual ~GFunctionWrapper() = default;


	bool is_valid() const _NO_EXCEPT_;

	explicit operator bool() const _NO_EXCEPT_;

	std::string_view get_name() const _NO_EXCEPT_;

	bool is_static() const _NO_EXCEPT_;

	const GTypeInfo* get_parent_type() const _NO_EXCEPT_;
	std::expected<GVariant, GFUNCTION_EXECUTION_ERROR> invoke(GVariant instance, std::vector<GVariant>& params)
	{

		if (params.size() == m_parameters.size())
		{
			//X TODO : CHECKING PARAMETERS TYPE

			return invokev_impl(instance,params);
		}

		return std::unexpected(GFUNCTION_EXECUTION_ERROR::GFUNCTION_EXECUTION_ERROR_PARAMETER_SIZE_INCORRECT);
	}
	std::expected<GVariant, GFUNCTION_EXECUTION_ERROR> invoke(GVariant instance)
	{
		if (0 == m_parameters.size())
		{
			//X TODO : CHECKING PARAMETERS TYPE

			return invoke_impl(instance);
		}

		return std::unexpected(GFUNCTION_EXECUTION_ERROR::GFUNCTION_EXECUTION_ERROR_PARAMETER_SIZE_INCORRECT);
	}

	std::expected<GVariant, GFUNCTION_EXECUTION_ERROR> invoke(GVariant instance, GVariant param1)
	{
		if (1 == m_parameters.size())
		{
			//X TODO : CHECKING PARAMETERS TYPE

			if (!param1.get_type().equals(m_parameters[0]))
			{
				return std::unexpected(GFUNCTION_EXECUTION_ERROR::GFUNCTION_EXECUTION_ERROR_ONE_OR_MORE_PARAMETER_TYPE_INCORRECT);
			}
			

			return invoke_impl(instance, param1);
		}

		return std::unexpected(GFUNCTION_EXECUTION_ERROR::GFUNCTION_EXECUTION_ERROR_PARAMETER_SIZE_INCORRECT);
	}

	std::expected<GVariant, GFUNCTION_EXECUTION_ERROR> invoke(GVariant instance, GVariant param1, GVariant param2)
	{
		if (2 == m_parameters.size())
		{
			//X TODO : CHECKING PARAMETERS TYPE

			if (!param1.get_type().equals(m_parameters[0]) || !param2.get_type().equals(m_parameters[1]))
			{
				return std::unexpected(GFUNCTION_EXECUTION_ERROR::GFUNCTION_EXECUTION_ERROR_ONE_OR_MORE_PARAMETER_TYPE_INCORRECT);
			}


			return invoke_impl(instance, param1,param2);
		}

		return std::unexpected(GFUNCTION_EXECUTION_ERROR::GFUNCTION_EXECUTION_ERROR_PARAMETER_SIZE_INCORRECT);
	}

	std::expected<GVariant, GFUNCTION_EXECUTION_ERROR> invoke(GVariant instance, GVariant param1, GVariant param2,GVariant param3)
	{
		if (3 == m_parameters.size())
		{
			//X TODO : CHECKING PARAMETERS TYPE

			if (!param1.get_type().equals(m_parameters[0]) || !param2.get_type().equals(m_parameters[1]) || !param3.get_type().equals(m_parameters[2]))
			{
				return std::unexpected(GFUNCTION_EXECUTION_ERROR::GFUNCTION_EXECUTION_ERROR_ONE_OR_MORE_PARAMETER_TYPE_INCORRECT);
			}
			return invoke_impl(instance, param1,param2,param3);
		}

		return std::unexpected(GFUNCTION_EXECUTION_ERROR::GFUNCTION_EXECUTION_ERROR_PARAMETER_SIZE_INCORRECT);
	}

	template<typename... Args>
	std::expected<GVariant, GFUNCTION_EXECUTION_ERROR> invokev(GVariant instance, Args&&... args);
protected:
	virtual std::expected<GVariant, GFUNCTION_EXECUTION_ERROR> invokev_impl(GVariant instance,std::vector<GVariant>& parameters) = 0;

	virtual std::expected<GVariant, GFUNCTION_EXECUTION_ERROR> invoke_impl(GVariant instance) = 0;
	virtual std::expected<GVariant, GFUNCTION_EXECUTION_ERROR> invoke_impl(GVariant instance, GVariant param1) = 0;
	virtual std::expected<GVariant, GFUNCTION_EXECUTION_ERROR> invoke_impl(GVariant instance, GVariant param1, GVariant param2) = 0;
	virtual std::expected<GVariant, GFUNCTION_EXECUTION_ERROR> invoke_impl(GVariant instance, GVariant param1, GVariant param2, GVariant param3) = 0;

protected:
	GType m_targetedType;
	std::string m_methodName;

	GType m_returnType;
	std::vector<GType> m_parameters;
};




template<typename ...Args>
inline std::expected<GVariant, GFUNCTION_EXECUTION_ERROR> GFunctionWrapper::invokev(GVariant instance, Args && ...args)
{
	if (sizeof...(Args) == m_parameters.size())
	{
		std::vector<GVariant> parameters{ GVariant(std::forward<Args>(args))... };
		//X TODO : CHECKING PARAMETERS TYPE

		for (int i = 0; i < parameters.size(); i++)
		{
			if (!parameters[i].get_type().equals(m_parameters[i]))
			{
				return std::unexpected(GFUNCTION_EXECUTION_ERROR::GFUNCTION_EXECUTION_ERROR_ONE_OR_MORE_PARAMETER_TYPE_INCORRECT);
			}
		}

		return invokev_impl(instance, parameters);
	}

	return std::unexpected(GFUNCTION_EXECUTION_ERROR::GFUNCTION_EXECUTION_ERROR_PARAMETER_SIZE_INCORRECT);
}



template <typename C, typename Return, typename... Args>
class GMemberFunctionWrapper : public GFunctionWrapper
{
	using mem_func_ptr = Return(C::*)(Args...);

public:
	GMemberFunctionWrapper(std::string_view name, mem_func_ptr ptr) : GFunctionWrapper(name,add_or_get_type_info<C>(),
		add_or_get_type_info<Return>(),	{ add_or_get_type_info<std::remove_cv_t<std::remove_reference_t<Args>>>()... })
	{
		m_memFunPtr = ptr;
	}

private:

	// If this method is invoked. Parameters count are same but we are not sure about types



	virtual std::expected<GVariant,GFUNCTION_EXECUTION_ERROR> invokev_impl(GVariant instance, std::vector<GVariant>& parameters) override
	{
		if (!instance.is_valid())
		{
			return std::unexpected(GFUNCTION_EXECUTION_ERROR::GFUNCTION_EXECUTION_ERROR_PARAMETER_INSTANCE_TYPE_INVALID);
		}
		//X This type of function is not support virtual and override
		if (!instance.is(m_targetedType))
		{
			return std::unexpected(GFUNCTION_EXECUTION_ERROR::GFUNCTION_EXECUTION_ERROR_PARAMETER_INSTANCE_TYPE_INCORRECT);
		}
	
		return unsafe_call(((C*)instance.get_raw()), parameters, std::make_index_sequence<sizeof...(Args)>{});
	}



	virtual std::expected<GVariant, GFUNCTION_EXECUTION_ERROR> invoke_impl(GVariant instance) override
	{
		if (!instance.is_valid())
		{
			return std::unexpected(GFUNCTION_EXECUTION_ERROR::GFUNCTION_EXECUTION_ERROR_PARAMETER_INSTANCE_TYPE_INVALID);
		}
		//X This type of function is not support virtual and override
		if (!instance.is(m_targetedType))
		{
			return std::unexpected(GFUNCTION_EXECUTION_ERROR::GFUNCTION_EXECUTION_ERROR_PARAMETER_INSTANCE_TYPE_INCORRECT);
		}

		std::vector<GVariant> params{  };

		return unsafe_call(((C*)instance.get_raw()), params, std::make_index_sequence<0>{});

	}

	virtual std::expected<GVariant, GFUNCTION_EXECUTION_ERROR> invoke_impl(GVariant instance, GVariant param1) override
	{
		if (!instance.is_valid())
		{
			return std::unexpected(GFUNCTION_EXECUTION_ERROR::GFUNCTION_EXECUTION_ERROR_PARAMETER_INSTANCE_TYPE_INVALID);
		}
		//X This type of function is not support virtual and override
		if (!instance.is(m_targetedType))
		{
			return std::unexpected(GFUNCTION_EXECUTION_ERROR::GFUNCTION_EXECUTION_ERROR_PARAMETER_INSTANCE_TYPE_INCORRECT);
		}

		std::vector<GVariant> params{ param1 };

		return unsafe_call(((C*)instance.get_raw()),params, std::make_index_sequence<1>{});

	}

	virtual std::expected<GVariant, GFUNCTION_EXECUTION_ERROR> invoke_impl(GVariant instance, GVariant param1, GVariant param2) override
	{
		if (!instance.is_valid())
		{
			return std::unexpected(GFUNCTION_EXECUTION_ERROR::GFUNCTION_EXECUTION_ERROR_PARAMETER_INSTANCE_TYPE_INVALID);
		}
		//X This type of function is not support virtual and override
		if (!instance.is(m_targetedType))
		{
			return std::unexpected(GFUNCTION_EXECUTION_ERROR::GFUNCTION_EXECUTION_ERROR_PARAMETER_INSTANCE_TYPE_INCORRECT);
		}

		std::vector<GVariant> params{ param1,param2 };

		return unsafe_call(((C*)instance.get_raw()), params, std::make_index_sequence<2>{});
	}
	virtual std::expected<GVariant, GFUNCTION_EXECUTION_ERROR> invoke_impl(GVariant instance, GVariant param1, GVariant param2, GVariant param3) override
	{
		if (!instance.is_valid())
		{
			return std::unexpected(GFUNCTION_EXECUTION_ERROR::GFUNCTION_EXECUTION_ERROR_PARAMETER_INSTANCE_TYPE_INVALID);
		}
		//X This type of function is not support virtual and override
		if (!instance.is(m_targetedType))
		{
			return std::unexpected(GFUNCTION_EXECUTION_ERROR::GFUNCTION_EXECUTION_ERROR_PARAMETER_INSTANCE_TYPE_INCORRECT);
		}
		std::vector<GVariant> params{ param1,param2,param3 };
		return unsafe_call(((C*)instance.get_raw()), params, std::make_index_sequence<3>{});
	}
	template <size_t... I>
	std::expected<GVariant, GFUNCTION_EXECUTION_ERROR> unsafe_call(C* rawPtr,std::vector<GVariant>& args,std::index_sequence<I...>, typename std::enable_if<sizeof...(I) == sizeof...(Args) && !std::is_void<Return>::value>::type* = 0)
	{
		std::tuple argsTuple = std::make_tuple(args[I].unsafe_as<std::remove_cv_t<std::remove_reference_t<Args>>>()...);
		Return var = (rawPtr->*m_memFunPtr)(*std::get<I>(argsTuple)...);
		return GVariant(var);
	}

	template <size_t... I>
	std::expected<GVariant, GFUNCTION_EXECUTION_ERROR> unsafe_call(C* rawPtr, std::vector<GVariant>& args, std::index_sequence<I...>, typename std::enable_if<sizeof...(I) == sizeof...(Args) && std::is_void<Return>::value>::type* = 0)
	{
		std::tuple argsTuple = std::make_tuple(args[I].unsafe_as<std::remove_cv_t<std::remove_reference_t<Args>>>()...);
		(rawPtr->*m_memFunPtr)(*std::get<I>(argsTuple)...);
		return GVariant();
	}

	template <size_t... I>
	std::expected<GVariant, GFUNCTION_EXECUTION_ERROR> unsafe_call(C* rawPtr, std::vector<GVariant>& args, std::index_sequence<I...>, typename std::enable_if<sizeof...(I) != sizeof...(Args)>::type* = 0)
	{
		return std::unexpected(GFUNCTION_EXECUTION_ERROR::GFUNCTION_EXECUTION_ERROR_PARAMETER_SIZE_INCORRECT);
	}


private:

	mem_func_ptr m_memFunPtr;
};

#endif // GMETHOD_WRAPPER_H
