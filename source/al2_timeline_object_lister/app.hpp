#pragma once

namespace apn::timeline_object_lister
{
	//
	// このクラスはアプリケーションです。
	//
	inline struct app_t : app_base_t, app_interface_t
	{
		//
		// プラグインの初期化処理を実行します。
		//
		virtual BOOL on_initialize_plugin(DWORD version) override
		{
			MY_TRACE_FUNC("");

			// 各種開始処理を実行します。
			debug.init();
			config_io.init();
			main_window.init(idd_main);

			// コンフィグをファイルから読み込みます。
			config_io.read();

			return TRUE;
		}

		//
		// プラグインの後始末処理を実行します。
		//
		virtual BOOL on_uninitialize_plugin() override
		{
			MY_TRACE_FUNC("");

			// 各種終了処理を実行します。
			main_window.exit();
			config_io.exit();
			debug.exit();

			return TRUE;
		}

		//
		// プラグインを登録します。
		//
		virtual BOOL on_register_plugin(HOST_APP_TABLE* host) override
		{
			MY_TRACE_FUNC("");

			// メインウィンドウを登録します。
			host->register_window_client(version_base->get_name().c_str(), main_window);

			// フォーカスオブジェクトが変更された時のリスナーです。
			const auto listener = [](void* param)
			{
				if (editor.handle->get_edit_state() != editor.handle->EDIT_STATE_EDIT) return;

				// オブジェクト一覧をリフレッシュします。
				app->refresh_list();
			};

			// イベントリスナーを登録します。
			// UPDATE_OBJECTはオブジェクトが移動したことを検出します。
			// CHANGE_EDIT_SCENEはシーンが切り替わったことを検出します。
			// CHANGE_FOCUS_OBJECTはフォーカスオブジェクトが切り替わったことを検出します。
			host->register_event_listener(EVENT_TYPE::UPDATE_OBJECT, nullptr, listener);
			host->register_event_listener(EVENT_TYPE::CHANGE_EDIT_SCENE, nullptr, listener);
			host->register_event_listener(EVENT_TYPE::CHANGE_FOCUS_OBJECT, nullptr, listener);
			host->register_event_listener(EVENT_TYPE::CHANGE_EDIT_SCENE, nullptr, [](void* param)
			{
				// 初回に1回だけ初期化後初期化処理を実行します。
				static BOOL is_initialized = FALSE;
				if (!is_initialized)
				{
					is_initialized = TRUE;

					effect_manager.post_init();
					color_manager.post_init();
				}
			});

			return TRUE;
		}

		//
		// オブジェクト一覧をリフレッシュするタイマーのIDです。
		//
		UINT_PTR refresh_timer_id = {};

		//
		// 編集セクションを使用して、
		// 現在のシーンにある全てのオブジェクトをリストアップします。
		//
		BOOL build_item_list_from_exedit_memory()
		{
			MY_TRACE_FUNC("");

			// 編集セクションを開始します。
			return editor.handle->call_edit_section([](EDIT_SECTION* edit)
			{
				app_impl.build_item_list_from_exedit_memory(edit);
			});
		}

		//
		// 編集セクションを使用して、
		// 現在のシーンにある全てのオブジェクトをリストアップします。
		//
		BOOL build_item_list_from_exedit_memory(EDIT_SECTION* edit)
		{
			MY_TRACE_FUNC("");

			// 編集情報が無効の場合は何もしません。
			if (edit->info->scale <= 0 || edit->info->rate <= 0) return FALSE;

			// フレーム数を時間に変換するための倍率を算出します。
			auto frame_to_sec_scale = (double)edit->info->scale / edit->info->rate;

			//
			// この関数は、文字列を追加します。
			//
			constexpr auto add_string = [](std::wstring& lhs, const std::wstring& rhs)
			{
				if (lhs.length()) lhs += L",";

				lhs += rhs;
			};

			//
			// この関数は、指定された値をデフォルトロケールで文字列化して返します。
			//
			const auto to_frame_string = [&](auto value) -> std::wstring
			{
				// 時間として表示する場合は
				if (hive.flag_as_time)
				{
					auto sec = value * frame_to_sec_scale;
					auto sec2 = (int)(sec * 100) % 100;
					auto sec1 = (int)sec % 60;
					auto min = (int)sec / 60 % 60;
					auto hour = (int)sec / 3600;

					// フレーム数を時間に変換して文字列化して返します。
					return my::format(L"{/02d}:{/02d}:{/02d}.{/02d}", hour, min, sec1, sec2);
				}
				else
				{
					// 文字列ストリームを作成します。
					std::wstringstream ss;

					// デフォルトロケールをセットします。
					ss.imbue(std::locale(""));

					// 数値をストリームに書き込みます。
					ss << value;

					// ストリームを文字列化して返します。
					return ss.str();
				}
			};

			//
			// この関数は、指定されたオブジェクトのレイヤーを文字列として返します。
			//
			const auto to_layer_string = [&](const auto& object, const OBJECT_LAYER_FRAME& object_layer_frame) -> std::wstring
			{
				// レイヤー番号を取得します。
				auto layer = object_layer_frame.layer;

				// レイヤー名を取得します。
				auto layer_name = edit->get_layer_name(layer);

				// 文字列化されたオブジェクトのレイヤーです。
				auto layer_string = my::format(L"{/}", layer + 1);

				// レイヤー名を追加します。
				add_string(layer_string, layer_name ? layer_name : L"");

				// レイヤーが非表示の場合は
				if (!edit->get_layer_enable(layer)) add_string(layer_string, L"非表示");

				// レイヤーがロックされている場合は
				if (edit->get_layer_lock(layer)) add_string(layer_string, L"ロック");

				// 文字列化されたオブジェクトのレイヤーを返します。
				return layer_string;
			};

			//
			// この関数は、指定されたオブジェクトのステータスを文字列として返します。
			//
			const auto to_status_string = [&](const auto& object, const OBJECT_LAYER_FRAME& object_layer_frame) -> std::wstring
			{
				// 文字列化されたオブジェクトのステータスです。
				auto status_string = std::wstring {};

//				if (edit->get_object_flag(object, OBJECT_FLAG_TYPE::ENABLE_GROUP)) add_string(status_string, L"グループ制御対象");
//				if (edit->get_object_flag(object, OBJECT_FLAG_TYPE::ENABLE_CAMERA)) add_string(status_string, L"カメラ制御対象");
				if (edit->get_object_flag(object, OBJECT_FLAG_TYPE::CLIPPING_OBJECT)) add_string(status_string, L"クリッピングオブジェクト");
				if (edit->get_object_flag(object, OBJECT_FLAG_TYPE::CLIPPING_UPPER_OBJECT)) add_string(status_string, L"上のオブジェクトでクリッピング");
				if (edit->get_object_section_num(object) > 1) add_string(status_string, L"中間点あり");

				// カーソルフレームと交差している場合は
				if (object_layer_frame.start <= edit->info->frame && edit->info->frame <= object_layer_frame.end) add_string(status_string, L"カーソル");

				// フォーカスオブジェクトの場合は
				if (object == edit->get_focus_object()) add_string(status_string, L"フォーカス");

				// 文字列化されたオブジェクトのステータスを返します。
				return status_string;
			};

			//
			// この関数は、指定されたオブジェクトの名前を文字列として返します。
			//
			const auto to_name_string = [&](const auto& object, const OBJECT_LAYER_FRAME& object_layer_frame, LPCWSTR effect_name) -> std::wstring
			{
				// オブジェクト名を取得できた場合は
				if (auto object_name = edit->get_object_name(object))
				{
					// オブジェクト名を返します。
					return object_name;
				}

				// エフェクト名が有効の場合は
				if (effect_name)
				{
					// エフェクト名を返します。
					return effect_name;
				}

				// それ以外の場合は空文字列を返します。
				return L"";
			};

			//
			// この関数は、指定されたオブジェクトの情報を文字列として返します。
			//
			const auto to_info_string = [&](const auto& object, const OBJECT_LAYER_FRAME& object_layer_frame) -> std::wstring
			{
				// エフェクトの数です。
				auto nb_effects = edit->get_effect_list(object, nullptr, 0);

				// エフェクトを取得します。
				auto effects = std::make_unique<EFFECT_HANDLE[]>(nb_effects);
				edit->get_effect_list(object, effects.get(), nb_effects);

				// 文字列化されたオブジェクトの情報です。
				auto info = my::format(L"{/}", nb_effects);

				// エフェクトを走査します。
				for (auto j = 0; j < nb_effects; j++)
				{
					// エフェクト名を文字列に追加します。
					add_string(info, edit->get_effect_name(effects[j]));
				}

				// 文字列化されたオブジェクトの情報を返します。
				return info;
			};

			// コレクションをリセットします。
			list.nodes.clear();
			finder.nodes.clear();

			// 全てのレイヤーを走査します。
			for (auto layer = 0; layer <= edit->info->layer_max; layer++)
			{
				// レイヤーのRGBAを取得します。
				auto layer_rgba = edit->get_layer_color(layer);

				// レイヤーの色です。
				COLORREF layer_color = CLR_DEFAULT;

				if (layer_rgba.a)
					layer_color = RGB(layer_rgba.r, layer_rgba.g, layer_rgba.b);

				// オブジェクトの検索位置です。
				auto frame = 0;

				// オブジェクトを走査します。
				while (auto object = edit->find_object(layer, frame))
				{
					// オブジェクトの位置を取得します。
					auto object_layer_frame = edit->get_object_layer_frame(object);

					// オブジェクトの検索位置を次に進めます。
					frame = object_layer_frame.end + 1;

					// 先頭付近のエフェクトを取得します。
					EFFECT_HANDLE effects[2] = {};
					auto nb_effects = edit->get_effect_list(object, effects, std::size(effects));
					if (!effects[0]) continue;

					// 先頭のエフェクト名を取得します。
					auto effect_name = edit->get_effect_name(effects[0]);
					if (!effect_name) continue;

					// フィルタオブジェクトかどうかのフラグです。
					auto flag_is_filter_object = FALSE;

					// 先頭のエフェクト名が"フィルタオブジェクト"の場合は
					if (nb_effects >= 2 && ::lstrcmpW(effect_name, L"フィルタオブジェクト") == 0)
					{
						// フィルタオブジェクトだったのでフラグを立てます。
						flag_is_filter_object = TRUE;

						// 2個目のエフェクト名を取得します。
						effect_name = edit->get_effect_name(effects[1]);
						if (!effect_name) continue;
					}

					// ノードを新規作成します。
					auto node = list.create_node(object_layer_frame, layer_color,
						color_manager.get_index(effect_name, flag_is_filter_object));

					// オブジェクトの開始位置と長さを算出します。
					auto start = object_layer_frame.start;
					auto range = object_layer_frame.end + 1 - object_layer_frame.start;

					// ノードの値をセットします。
					node->values[list.k_column.k_layer] = to_layer_string(object, object_layer_frame);
					node->values[list.k_column.k_status] = to_status_string(object, object_layer_frame);
					node->values[list.k_column.k_name] = to_name_string(object, object_layer_frame, effect_name);
					node->values[list.k_column.k_info] = to_info_string(object, object_layer_frame);
					node->values[list.k_column.k_start] = to_frame_string(start);
					node->values[list.k_column.k_range] = to_frame_string(range);
				}
			}

			return TRUE;
		}

		//
		// オブジェクト一覧を絞り込みます。
		//
		virtual BOOL apply_finder() override
		{
			MY_TRACE_FUNC("");

			// オブジェクト一覧を絞り込みます。
			finder.apply();

			// ダイアログのリストビューをリフレッシュします。
			listview.to_ui();

			return TRUE;
		}

		//
		// オブジェクト一覧をリフレッシュします。
		//
		virtual BOOL refresh_list() override
		{
			MY_TRACE_FUNC("");

			// ダイアログが表示されていない場合は何もしません。
			if (!::IsWindowVisible(main_window)) return FALSE;

			// タイマーを開始します。
			refresh_timer_id = ::SetTimer(main_window, refresh_timer_id, hive.refresh_delay,
				[](HWND hwnd, UINT message, UINT_PTR timer_id, DWORD time)
			{
				// オブジェクト一覧を構築します。
				app_impl.build_item_list_from_exedit_memory();

				// オブジェクト一覧をソートします。
				sort.apply();

				// オブジェクト一覧を絞り込みます。
				finder.apply();

				// ダイアログのリストビューをリフレッシュします。
				listview.to_ui();

				// ヘッダーコントロールをリフレッシュします。
				header.to_ui();

				// タイマーを終了します。
				::KillTimer(main_window, app_impl.refresh_timer_id), app_impl.refresh_timer_id = {};
			});

			return TRUE;
		}

		//
		// オブジェクト一覧をリセットします。
		//
		virtual BOOL reset_list() override
		{
			MY_TRACE_FUNC("");

			// ダイアログが表示されていない場合は何もしません。
			if (!::IsWindowVisible(main_window)) return FALSE;

			// コレクションをリセットします。
			list.nodes.clear();
			finder.nodes.clear();

			// ダイアログのリストビューをリフレッシュします。
			listview.to_ui();

			// ヘッダーコントロールをリフレッシュします。
			header.to_ui();

			return TRUE;
		}

		//
		// オブジェクト一覧のソートをリセットします。
		//
		virtual BOOL reset_sort() override
		{
			MY_TRACE_FUNC("");

			// ソートをリセットします。
			sort.nodes.clear();
			main_window.reset_columns();

			// オブジェクト一覧をソートします。
			sort.apply();

			// オブジェクト一覧を絞り込みます。
			finder.apply();

			// ダイアログのリストビューをリフレッシュします。
			listview.to_ui();

			return TRUE;
		}

		//
		// オブジェクト一覧の絞り込みをリセットします。
		//
		virtual BOOL reset_finder() override
		{
			MY_TRACE_FUNC("");

			// 絞り込みをリセットします。
			finder.reset();

			// オブジェクト一覧を絞り込みます。
			finder.apply();

			// ダイアログのリストビューをリフレッシュします。
			listview.to_ui();

			// ヘッダーコントロールをリフレッシュします。
			header.to_ui();

			return TRUE;
		}

		//
		// 指定されたオブジェクトをフォーカスオブジェクトにします。
		//
		virtual BOOL focus_to(int item_index, int sub_item_index) override
		{
			MY_TRACE_FUNC("{/}, {/}", item_index, sub_item_index);

			// 項目のインデックスが範囲外の場合は何もしません。
			if ((size_t)item_index >= finder.nodes.size()) return FALSE;
			if ((size_t)sub_item_index >= (size_t)list.k_column.k_max_size) return FALSE;

			// 編集セクションを開始します。
			return editor.handle->call_edit_section_param(
				(void*)(size_t)item_index, [](void* p, EDIT_SECTION* edit)
			{
				// ノードを取得します。
				const auto& node = finder.nodes[(size_t)p];

				// オブジェクトを取得出来た場合は
				if (auto object = edit->find_object(
					node->object_layer_frame.layer, node->object_layer_frame.start))
				{
					// オブジェクトをフォーカスオブジェクトにします。
					edit->set_focus_object(object);
				}
			});
		}

		//
		// 指定されたオブジェクトが表示されるように、
		// タイムラインをスクロールします。
		//
		virtual BOOL scroll_to(int item_index, int sub_item_index) override
		{
			MY_TRACE_FUNC("{/}, {/}", item_index, sub_item_index);

			// 項目のインデックスが範囲外の場合は何もしません。
			if ((size_t)item_index >= finder.nodes.size()) return FALSE;
			if ((size_t)sub_item_index >= (size_t)list.k_column.k_max_size) return FALSE;

			// 編集セクションを開始します。
			return editor.handle->call_edit_section_param(
				(void*)(size_t)item_index, [](void* p, EDIT_SECTION* edit)
			{
				// ノードを取得します。
				const auto& node = finder.nodes[(size_t)p];

				// タイムラインの水平方向スクロール位置を算出します。
				auto frame = node->object_layer_frame.start;
				frame -= edit->info->display_frame_num / 2;

				// タイムラインの垂直方向スクロール位置を算出します。
				auto layer = node->object_layer_frame.layer;
				layer -= edit->info->display_layer_num / 2;

				// タイムラインをスクロールします。
				edit->set_display_layer_frame(layer, frame);
			});
		}
	} app_impl;
}
