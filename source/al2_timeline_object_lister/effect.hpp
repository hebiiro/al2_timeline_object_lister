#pragma once

namespace apn::timeline_object_lister
{
	//
	// このクラスはエフェクト情報を管理します。
	//
	inline struct effect_manager_t
	{
		//
		// このクラスはエフェクト情報です。
		//
		struct node_t {
			int type;
			int flag;
		};

		//
		// エフェクト情報のコレクションです。
		// キーはエフェクト名です。
		//
		std::unordered_map<std::wstring, node_t> nodes;

		//
		// 初期化後初期化処理を実行します。
		//
		BOOL post_init()
		{
			MY_TRACE_FUNC("");

			// 編集ハンドルが無効の場合は何もしません。
			if (!editor.handle) return FALSE;

			// エフェクト名を列挙します。
			editor.handle->enum_effect_name(
				this, [](void* param, LPCWSTR name, int type, int flag)
			{
//				MY_TRACE("{/}\t\t\t{/hex}\t{/hex}\n", name, type, flag);

				auto p = (effect_manager_t*)param;

				// エフェクト情報をコレクションに追加します。
				p->nodes[name] = { type, flag };
			});

			return TRUE;
		}

		//
		// 指定されたエフェクト名に対応するエフェクト情報を返します。
		//
		node_t get_effect_info(const std::wstring& effect_name)
		{
			// エフェクト情報を返します。
			return nodes[effect_name];
		}
	} effect_manager;
}
