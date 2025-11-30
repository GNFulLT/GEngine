#include "../GObject/include/gobject/gobject_utils.h"
#include "include/engine\gproject.h"
#include "include/engine\scene\component\transform_component.h"



GOBJECT_ENABLE(GProject)
	GOBJECT_DEFINE_PROPERTY("projectName",&GProject::m_projectName)
	GOBJECT_DEFINE_PROPERTY("projectNamespace",&GProject::m_defaultNamespace)
	GOBJECT_DEFINE_PROPERTY("scriptBinaryPath",&GProject::m_scriptBinaryPath)
}
GOBJECT_ENABLE(TransformComponent)
	GOBJECT_DEFINE_PROPERTY_GS("position",&TransformComponent::m_position,&TransformComponent::position_getter,&TransformComponent::position_setter)
	GOBJECT_DEFINE_PROPERTY_GS("scale",&TransformComponent::m_scale,&TransformComponent::scale_getter,&TransformComponent::scale_setter)
	GOBJECT_DEFINE_PROPERTY("rotation",&TransformComponent::m_rotation)
}
