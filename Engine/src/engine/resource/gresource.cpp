#include "engine/resource/igresource_loader.h"
#include "engine/resource/iresource.h"


bool IResource::before_load()
{
	auto stage = m_loadingState.load();
	if (stage != RESOURCE_LOADING_STATE_UNLOADED)
	{
		return false;
	}

	stage = RESOURCE_LOADING_STATE_PREPARING;
	m_loadingState.exchange(stage);

	// Call prepare
	return prepare();
}

RESOURCE_INIT_CODE IResource::load()
{
	bool prepared = before_load();
	if (!prepared)
	{
		auto stage = m_loadingState.load();
		if (stage != RESOURCE_LOADING_STATE_UNLOADED)
		{
			return RESOURCE_INIT_CODE_UNKNOWN_EX;
		}
		else if (stage == RESOURCE_LOADING_STATE_PREPARING)
		{
			m_loadingState.store(RESOURCE_LOADING_STATE_UNLOADED);
		}
		return RESOURCE_INIT_CODE_UNKNOWN_EX;
	}

	m_loadingState.store(RESOURCE_LOADING_STATE_LOADING);
	
	auto code = load_impl();
	
	if(code != RESOURCE_INIT_CODE_OK)
	{
		// Uunprepare and return false
		m_loadingState.store(RESOURCE_LOADING_STATE_UNPREPARING);
		unprepare();
		return code;
	}


	// After load

	after_load();

	m_loadingState.store(RESOURCE_LOADING_STATE_LOADED);

	return RESOURCE_INIT_CODE_OK;
}

void IResource::after_load()
{
}


void IResource::before_unload()
{
	
}

void IResource::unload()
{
	auto stage = m_loadingState.load();
	if (stage != RESOURCE_LOADING_STATE_LOADED)
	{
		return;
	}
	m_loadingState.store(RESOURCE_LOADING_STATE_UNLOADING);

	before_unload();

	unload();

	m_loadingState.store(RESOURCE_LOADING_STATE_UNPREPARING);

	after_unload();
	
	m_loadingState.store(RESOURCE_LOADING_STATE_UNLOADED);

}

void IResource::after_unload()
{
	unprepare();
}

bool IResource::prepare()
{
	return RESOURCE_INIT_CODE_OK == prepare_impl();
}

void IResource::unprepare()
{
	unprepare_impl();
}
