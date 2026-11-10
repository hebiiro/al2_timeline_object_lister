#pragma once

namespace apn::timeline_object_lister
{
	//
	// このクラスはアプリケーションのインターフェイスです。
	//
	inline struct app_interface_t {
		//
		// コンストラクタです。
		//
		app_interface_t() { app = this; }

		virtual BOOL apply_finder() = 0;
		virtual BOOL refresh_list() = 0;
		virtual BOOL reset_list() = 0;
		virtual BOOL reset_sort() = 0;
		virtual BOOL reset_finder() = 0;
		virtual BOOL focus_to(int item_index, int sub_item_index) = 0;
		virtual BOOL scroll_to(int item_index, int sub_item_index) = 0;
	} *app = nullptr;
}
