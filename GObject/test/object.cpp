#include <gtest/gtest.h>
#include "gobject/gobject.h"
#include "first.h"
#include "gobject/gvariant.h"
#include "public/core/templates/shared_ptr.h"
#include <iostream>
#include "gobject/gproperty.h"

TEST(GObjectDBTest, Register)
{
	First first;
	first.number = 6;
	GType type = GTypeUtils::get_type_of<First>();
	auto props = type.get_properties();
	int a = 5;
	
	std::cin >> x;
}