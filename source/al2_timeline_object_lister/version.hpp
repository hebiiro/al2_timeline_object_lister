#pragma once

namespace apn::timeline_object_lister
{
	//
	// このクラスはこのプログラムのバージョンを管理します。
	//
	inline struct version_t : version_base_t {
		//
		// コンストラクタです。
		//
		version_t() : version_base_t(L"リストアップ", L"🐍リストアップ🔖", L"r1") {}
	} version;
}
