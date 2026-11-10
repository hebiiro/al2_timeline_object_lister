#pragma once

namespace apn::timeline_object_lister
{
	//
	// このクラスはアドインダイアログのリストビューです。
	//
	inline struct listview_t : my::Window
	{
		//
		// ダミーのイメージのサイズです。
		//
		inline static constexpr SIZE k_image_size = { 36, 1 };

		//
		// 最後のホット項目です。
		//
		int last_hot_item = -1;

		//
		// ダミーのイメージリストです。
		//
		my::image_list::unique_ptr imagelist;

		//
		// リストビューを初期化します。
		//
		BOOL init(HWND hwnd)
		{
			MY_TRACE_FUNC("");

			// リストビューをサブクラス化します。
			if (!__super::subclass(hwnd)) return FALSE;

			// リストビューのウィンドウスタイルを変更します。
			my::modify_style(*this, 0, WS_CLIPSIBLINGS | WS_CLIPCHILDREN);

			// 拡張リストビュースタイルをセットします。
			ListView_SetExtendedListViewStyle(
				*this,
				LVS_EX_DOUBLEBUFFER |
				LVS_EX_FULLROWSELECT |
				LVS_EX_GRIDLINES |
				LVS_EX_HEADERDRAGDROP |
//				LVS_EX_HEADERINALLVIEWS |
//				LVS_EX_INFOTIP |
//				LVS_EX_LABELTIP |
//				LVS_EX_TRACKSELECT |
//				LVS_EX_ONECLICKACTIVATE |
//				LVS_EX_UNDERLINEHOT |
				LVS_EX_SUBITEMIMAGES |
				0);

			// ツールチップを取得します。
			auto tooltip = ListView_GetToolTips(*this);

			// ツールチップのウィンドウスタイルを変更します。
			my::modify_style(tooltip, 0, TTS_NOPREFIX);

			// ツールチップの最大幅を変更します。
			// これにより、複数行のツールチップを表示できるようになります。
			::SendMessage(tooltip, TTM_SETMAXTIPWIDTH, 0, 1920);

			// ダミーのイメージリストを作成します。
			imagelist.reset(::ImageList_Create(
				k_image_size.cx, k_image_size.cy, ILC_COLOR32 | ILC_MASK, 1, 1));

			// 32bit DIBを作成します。
			BITMAPINFO bmi = {};
			bmi.bmiHeader.biSize = sizeof(bmi.bmiHeader);
			bmi.bmiHeader.biWidth = k_image_size.cx;
			bmi.bmiHeader.biHeight = k_image_size.cy;
			bmi.bmiHeader.biPlanes = 1;
			bmi.bmiHeader.biBitCount = 32;
			bmi.bmiHeader.biCompression = BI_RGB;
			BYTE* bits = {};
			my::gdi<HBITMAP>::unique_ptr bitmap(
				::CreateDIBSection(nullptr, &bmi, DIB_RGB_COLORS, (void**)&bits, nullptr, 0));

			// 32bit DIBをイメージリストに追加します。
			::ImageList_Add(imagelist.get(), bitmap.get(), nullptr);

			// イメージリストをセットします。
			ListView_SetImageList(*this, imagelist.get(), LVSIL_SMALL);

			return TRUE;
		}

		//
		// 絞り込み済みノードのリストをリストビューに適用します。
		//
		BOOL to_ui()
		{
			MY_TRACE_FUNC("");

			// 絞り込み済みノードの数をリストビューの項目数にセットします。
			ListView_SetItemCountEx(*this, finder.nodes.size(), LVSICF_NOSCROLL);

			return TRUE;
		}

		//
		// リストビューのウィンドウプロシージャです。
		//
		virtual LRESULT on_wnd_proc(HWND hwnd, UINT message, WPARAM w_param, LPARAM l_param) override
		{
			switch (message)
			{
			case WM_MOUSEMOVE:
				{
//					MY_TRACE_FUNC("{/}, {/hex}, {/hex}", my::message_to_string(message), w_param, l_param);

					// スコープ終了時の処理です。
					my::scope_exit scope_exit([&]()
					{
						// ホット項目を取得します。
						auto hot_item = ListView_GetHotItem(*this);
//						MY_TRACE_INT(hot_item);

						// ホット項目が変わっていない場合は何もしません。
						if (last_hot_item == hot_item) return;

						// 最後のホット項目を更新します。
						last_hot_item = hot_item;

						// ホット項目をハイライト表示します。
						ListView_SetItemState(*this, hot_item,
							LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);

						// ツールチップを取得します。
						auto tooltip = ListView_GetToolTips(*this);

						// ホット項目のインデックスが範囲外の場合は
						if ((size_t)hot_item >= finder.nodes.size())
						{
							// ツールチップを非アクティブ化します。
							::SendMessage(tooltip, TTM_ACTIVATE, FALSE, 0);

							return;
						}

						// ツールチップをアクティブ化します。
						::SendMessage(tooltip, TTM_ACTIVATE, TRUE, 0);

						// ツールチップテキストです。
						std::wstring tooltip_text;

						// 列を走査します。
						for (int32_t i = 0; i < list.k_column.k_max_size; i++)
						{
							// ノードのテキストをツールチップテキストに追加します。
							tooltip_text += list.k_column.k_datas[i].text;
							tooltip_text += L" ";
							tooltip_text += finder.nodes[hot_item]->get_text(i) + L"\r\n";
						}

						// ツールチップのテキストを変更(表示)します。
						TOOLINFO ti { sizeof(ti) };
						ti.uFlags = TTF_IDISHWND | TTF_ABSOLUTE;
						ti.hinst = hive.instance;
						ti.hwnd = *this;
						ti.lpszText = tooltip_text.data();
						::SendMessage(tooltip, TTM_UPDATETIPTEXT, 0, (LPARAM)&ti);
					});

					return __super::on_wnd_proc(hwnd, message, w_param, l_param);
				}
			}

			return __super::on_wnd_proc(hwnd, message, w_param, l_param);
		}
	} listview;
}
