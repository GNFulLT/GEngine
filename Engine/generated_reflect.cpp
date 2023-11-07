#include "C:\Users\lenovo\Desktop\GEngine\GObject\include\gobject\gobject_utils.h"
#include "C:\Users\lenovo\Desktop\GEngine\Engine\internal\engine\scene\component\transform_component.h"



GOBJECT_ENABLE(TransformComponent)
	GOBJECT_DEFINE_PROPERTY("m_localTransform",&TransformComponent::m_localTransform)
	GOBJECT_DEFINE_PROPERTY("globalTransform",&TransformComponent::m_globalTransform)
}
