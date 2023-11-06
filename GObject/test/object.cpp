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
	for (auto prop : *props)
	{
		auto pp = prop.get();
		auto ftft = pp->get(first).value();
		auto getVal = *((int*)ftft.get_raw());
		std::cout << pp->get_name();
		auto tp = pp->get(first);
		if (tp.has_value())
		{
			auto val = tp.value();
			auto num = (int*)val.get_raw();
			auto fff = *num;
		}
	}
	int x;
	std::cin >> x;
}