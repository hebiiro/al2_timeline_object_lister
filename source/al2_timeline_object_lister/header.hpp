#pragma once

namespace apn::timeline_object_lister
{
	//
	// このクラスはヘッダーコントロールです。
	//
	inline struct header_t : my::Window
	{
		//
		// ヘッダーコントロールを保有するリストビューです。
		//
		HWND listview = {};

		//
		// ヘッダーコントロールを初期化します。
		//
		BOOL init(HWND listview)
		{
			MY_TRACE_FUNC("");

			// ヘッダーコントロールをサブクラス化します。
			if (!__super::subclass(ListView_GetHeader(listview))) return FALSE;

			// ヘッダーコントロールを保有するリストビューを取得しておきます。
			this->listview = listview;

			// ヘッダーコントロールのウィンドウスタイルを変更します。
			my::modify_style(*this, 0, HDS_FILTERBAR);

			// 列データを走査します。
			for (auto i = 0; i < list.k_column.k_max_size; i++)
			{
				// 列データを取得します。
				const auto& data = list.k_column.k_datas[i];

				// リストビューに列を追加します。
				lv::insert_column(listview, i, data.text, data.width);
			}

			return TRUE;
		}

		//
		// 絞り込み用テキストをヘッダーコントロールのフィルタテキストに適用します。
		//
		BOOL to_ui()
		{
			MY_TRACE_FUNC("");

			// 列を走査します。
			for (auto i = 0; i < list.k_column.k_max_size; i++)
			{
				// フィルタテキストをセットします。
				lv::set_filter_text(*this, i, finder.texts[i]);
			}

			return TRUE;
		}

		//
		// ヘッダーコントロールのフィルタテキストを絞り込み用テキストに適用します。
		//
		BOOL from_ui()
		{
			MY_TRACE_FUNC("");

			// 列を走査します。
			for (auto i = 0; i < list.k_column.k_max_size; i++)
			{
				// フィルタテキストを取得します。
				auto text = lv::get_filter_text(*this, i);

				// 絞り込み用テキストをセットします。
				finder.texts[i] = text;
			}

			return TRUE;
		}

		//
		// 絞り込みメニューを表示します。
		//
		BOOL show_filter_menu(int32_t col_index, LPCRECT rc)
		{
			MY_TRACE_FUNC("{/}", col_index);

			if ((size_t)col_index >= (size_t)list.k_column.k_max_size) return FALSE;

			// ポップアップメニューを作成します。
			my::menu::unique_ptr menu(::CreatePopupMenu());

			// 絞り込み用テキストを格納するコンテナです。
			// std::setなので、重複テキストは除外されます。
			std::set<std::wstring> texts;

			// コンテナに空文字列を追加します。
			texts.emplace(L"");

			// コンテナに絞り込み用テキストを追加します。
			for (const auto& node : list.nodes)
			{
				// 文字列ストリームを作成します。
				std::wstringstream stream(node->get_text(col_index));

				// 空白で区切ってコンテナに追加します。
				std::wstring item;
				while (std::getline(stream, item, L','))
					texts.emplace(item);
			}

			// コンテナに格納されたテキストをポップアップメニューに追加します。
			{
				auto i = (size_t)1;
				for (const auto& text : texts)
					::AppendMenu(menu.get(), MF_STRING, i++, text.c_str());
			}

			// ポップアップメニューを表示します。
			auto point = POINT { rc->right, rc->bottom };
			my::map_window_points(*this, nullptr, &point);
			auto id = ::TrackPopupMenuEx(menu.get(),
				TPM_NONOTIFY | TPM_RETURNCMD |
				TPM_RIGHTALIGN | TPM_TOPALIGN,
				point.x, point.y, *this, nullptr);
			if (!id) return FALSE;

			// ポップアップメニューから絞り込み用テキストを取得します。
			auto text = my::get_menu_item_text(menu.get(), id, MF_BYCOMMAND);

			// 絞り込み用テキストをヘッダーコントロールにセットします。
			lv::set_filter_text(*this, col_index, text);

			// 絞り込み用テキストをセットします。
			finder.texts[col_index] = text;

			// 絞り込みを適用します。
			app->apply_finder();

			return TRUE;
		}
	} header;
}
