#pragma once

namespace apn::timeline_object_lister
{
	//
	// このクラスは他クラスから共通して使用される変数を保持します。
	//
	inline struct hive_t : hive_base_t
	{

		//
		// TRUEの場合は位置や範囲を時間として表示します。
		//
		BOOL flag_as_time = FALSE;

		//
		// リストをリフレッシュするまでの遅延時間です。
		//
		int refresh_delay = 20;
	} hive;
}
