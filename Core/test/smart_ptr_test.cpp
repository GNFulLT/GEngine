#include "public/core/string/gstring.h"
#include "public/core/templates/shared_ptr.h"
#include <gtest/gtest.h>
#include "public/core/templates/signal/gslot.h"

void s(int a)
{
	int c = 5;
	int d = 6;
}

void s2(int a)
{
	int f = 6;
	int g = 7;
}

TEST(GSignal, SignalConnect)
{
	GSlot<void(int)> slot;

	auto fnc = std::bind(&s, std::placeholders::_1);
	fnc(5);
	{
		auto signal = slot.connect(fnc);
	}
	auto signal2 = slot.connect(std::bind(&s2, std::placeholders::_1));
	slot(5);
}

//TEST(WeakPtrTest, All)
//{
//	GWeakPtr<GString> weak;
//	
//	{
//		GSharedPtr<GString> ptr(new GString(_T("Hello")));
//		weak = ptr.to_weak_ptr();
//		EXPECT_EQ(weak.is_valid(), true);
//
//		auto ptr2 = weak.as_shared();
//	}
//	bool valid = weak.is_valid();
//	auto ptr3 = weak.as_shared();
//	bool valid2 = ptr3.is_valid();
//	EXPECT_EQ(valid, false);
//
//}

//
//TEST(SharedPtrTest, RefCountAfterCopyDeleteCtor)
//{
//	GSharedPtr<GString> ptr(new GString(_T("Hello")));
//	
//	EXPECT_EQ(ptr.get_shared_ref_count(), 1);
//	
//	EXPECT_EQ(ptr.get_weak_ref_count(), 1);
//
//	{
//		GSharedPtr<GString> ptr2 = ptr;
//	
//		EXPECT_EQ(ptr.get_shared_ref_count(), 2);
//	}
//
//	EXPECT_EQ(ptr.get_shared_ref_count(), 1);
//}
//
//TEST(SharedPtrTest, RefCountAfterCopyDeleteCtor_ThreadSafe)
//{
//	GSharedPtr<GString,GSHARED_PTR_INTERNAL_MODE_THREAD_SAFE> ptr(new GString(_T("Hello")));
//
//	EXPECT_EQ(ptr.get_shared_ref_count(), 1);
//
//	EXPECT_EQ(ptr.get_weak_ref_count(), 1);
//
//	{
//		GSharedPtr<GString, GSHARED_PTR_INTERNAL_MODE_THREAD_SAFE> ptr2 = ptr;
//
//		EXPECT_EQ(ptr.get_shared_ref_count(), 2);
//	}
//
//	EXPECT_EQ(ptr.get_shared_ref_count(), 1);
//}
//
//TEST(GStringTest, GStringWSTRTOWSTR)
//{
//	GString str(_T("HELLO"));
//	std::wstring expected = L"HELLO";
//	bool same = _wcsicmp(str.get_data(), expected.c_str()) == 0;
//	EXPECT_TRUE(same);
//
//}
//
//TEST(GStringTest, GStringASCIITOWSTR)
//{
//	GString str("HELLOÞ");
//	std::wstring expected = L"HELLOÞ";
//	bool same = _wcsicmp(str.get_data(), expected.c_str()) == 0;
//	EXPECT_TRUE(same);
//}
