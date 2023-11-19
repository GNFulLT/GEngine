#include "C:\Users\lenovo\Desktop\GEngine\GObject\include\gobject\gobject_utils.h"
#include "C:\Users\lenovo\Desktop\GEngine\Engine\include\engine\gproject.h"
#include "C:\Users\lenovo\Desktop\GEngine\Engine\internal\engine\scene\component\transform_component.h"



GOBJECT_ENABLE(GProject)
	GOBJECT_DEFINE_PROPERTY("projectName",&GProject::m_projectName)
	GOBJECT_DEFINE_PROPERTY("projectNamespace",&GProject::m_defaultNamespace)
}
GOBJECT_ENABLE(TransformComponent)
	GOBJECT_DEFINE_PROPERTY_GS("position",&TransformComponent::m_position,&TransformComponent::position_getter,&TransformComponent::position_setter)
	GOBJECT_DEFINE_PROPERTY_GS("scale",&TransformComponent::m_scale,&TransformComponent::scale_getter,&TransformComponent::scale_setter)
	GOBJECT_DEFINE_PROPERTY("rotation",&TransformComponent::m_rotation)
}
