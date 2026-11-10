#pragma once

namespace apn::timeline_object_lister
{
	//
	// このクラスはメインウィンドウです。
	//
	inline struct main_window_t : my::layout::Window<config_dialog_base_t>
	{
		//
		// タイマーのIDです。現在は使用されていません。
		//
		inline static constexpr struct timer_id_t {
			inline static constexpr auto k_refresh_list = WM_APP + 2026;
		} k_timer_id;

		//
		// コントロールを初期化します。
		//
		BOOL init_controls()
		{
			MY_TRACE_FUNC("");

			// リストビューを初期化します。
			if (!listview.init(ctrl(idc_list))) return FALSE;

			// インプレースエディットボックスを初期化します。
			if (!inplace_edit.init(listview, ctrl(idc_inplace_editbox))) return FALSE;

			// ヘッダーコントロールを初期化します。
			if (!header.init(listview)) return FALSE;

			return TRUE;
		}

		//
		// リストビューの列名を変更します。
		//
		BOOL set_column_text(int32_t col_index)
		{
			MY_TRACE_FUNC("{/}", col_index);

			// 列データから列名を取得します。
			auto text = (std::wstring)list.k_column.k_datas[col_index].text;

			// ソートノードの番号です。
			auto index = (int32_t)1;

			// ソートノードを走査します。
			for (const auto& node : sort.nodes)
			{
				// 指定された列のソートノードの場合は
				if (node->col_index == col_index)
				{
					// ソート情報を列名に加えます。
					text += node->ascendant ? L"△" : L"▽";
					text += my::format(L"{/}", index);

					// ループを終了します。
					break;
				}

				// ソートノードの番号を増やします。
				index++;
			}

			// リストビューの列名を変更します。
			LVCOLUMN col = {
				.mask = LVCF_TEXT,
				.pszText = text.data(),
			};
			return ListView_SetColumn(listview, col_index, &col);
		}

		//
		// リストビューの項目をソートします。
		//
		BOOL sort_items(int32_t col_index)
		{
			MY_TRACE_FUNC("{/}", col_index);

			// ソートノードを追加します。
			sort.add(col_index);

			// リストをソートします。
			sort.apply();

			// リストを絞り込みます。
			finder.apply();

			// ヘッダーのテキストを更新します。
			for (int32_t i = 0; i < list.k_column.k_max_size; i++)
				set_column_text(i);

			// リストビューをリフレッシュします。
			listview.to_ui();

			return TRUE;
		}

		//
		// カラムをリセットします。
		//
		BOOL reset_columns()
		{
			MY_TRACE_FUNC("");

			// 列データを走査します。
			for (auto i = 0; i < list.k_column.k_max_size; i++)
			{
				// 列データを取得します。
				const auto& data = list.k_column.k_datas[i];

				// リストビューの列名を変更します。
				LVCOLUMN col = {
					.mask = LVCF_TEXT,
					.pszText = (LPWSTR)data.text,
				};
				ListView_SetColumn(listview, i, &col);
			}

			return TRUE;
		}

		//
		// リストビューのコンテキストメニューを表示します。
		//
		BOOL show_context_menu(int32_t index, int32_t sub_item)
		{
			MY_TRACE_FUNC("{/}, {/}", index, sub_item);

			// ポップアップメニューの項目IDです。
			constexpr uint32_t k_flag_as_time = 1000;
			constexpr uint32_t k_refresh = 1100;
			constexpr uint32_t k_reset_sort = 1101;
			constexpr uint32_t k_reset_finder = 1102;

			// ポップアップメニューを作成します。
			my::menu::unique_ptr menu(::CreatePopupMenu());

			// ポップアップメニューに項目を追加します。
			::AppendMenu(menu.get(), MF_STRING, k_flag_as_time, _T("フレームを時間で表示"));
			::AppendMenu(menu.get(), MF_SEPARATOR, 0, nullptr);
			::AppendMenu(menu.get(), MF_STRING, k_refresh, _T("リフレッシュ"));
			::AppendMenu(menu.get(), MF_STRING, k_reset_sort, _T("並び順をリセット"));
			::AppendMenu(menu.get(), MF_STRING, k_reset_finder, _T("絞り込みをリセット"));

			// 変数の状態に合わせて、ポップアップメニューの項目にチェックマークを入れます。
			if (hive.flag_as_time) ::CheckMenuItem(menu.get(), k_flag_as_time, MF_CHECKED);

			// ポップアップメニューを表示します。
			auto point = my::get_cursor_pos();
			auto id = ::TrackPopupMenuEx(menu.get(),
				TPM_NONOTIFY | TPM_RETURNCMD, point.x, point.y, listview, nullptr);
			if (!id) return FALSE;

			switch (id)
			{
			case k_flag_as_time:
				{
					// 時間で表示するフラグを切り替えます。
					hive.flag_as_time = !hive.flag_as_time;

					// リストをリフレッシュします。
					app->refresh_list();

					break;
				}
			case k_refresh:
				{
					// リストをリフレッシュします。
					app->refresh_list();

					break;
				}
			case k_reset_sort:
				{
					// ソートをリセットします。
					app->reset_sort();

					break;
				}
			case k_reset_finder:
				{
					// 絞り込みをリセットします。
					app->reset_finder();

					break;
				}
			}

			return TRUE;
		}

		//
		// ダイアログのレイアウトを更新します。
		//
		BOOL recalc_layout()
		{
			MY_TRACE_FUNC("");

			// aviutl2の場合は、WM_INITDIALOGより先にWM_SIZEが呼ばれるので、
			// コントロールが初期化済みかチェックする必要があります。
			if (!listview) return FALSE;

			// ダイアログのクライアント矩形を取得します。
			auto client_rc = my::get_client_rect(*this);

			my::DeferWindowPos dwp(1);

			// リストビューのウィンドウ位置を変更します。
			dwp.set_window_pos(listview, nullptr, &client_rc, SWP_NOZORDER | SWP_NOACTIVATE);

			return TRUE;
		}

		//
		// ダイアログの初期化処理です。
		//
		virtual void on_init_dialog() override
		{
			MY_TRACE_FUNC("");

			init_controls();
		}

		//
		// ダイアログのコマンド処理です。
		//
		virtual void on_command(UINT code, UINT control_id, HWND control) override
		{
			MY_TRACE_FUNC("{/hex}, {/hex}, {/hex}", code, control_id, control);
		}

		//
		// ウィンドウプロシージャです。
		//
		virtual LRESULT on_wnd_proc(HWND hwnd, UINT message, WPARAM w_param, LPARAM l_param) override
		{
			switch (message)
			{
			case WM_TIMER:
				{
					switch (w_param)
					{
					case k_timer_id.k_refresh_list:
						{
							// リストをリフレッシュします。
							app->refresh_list();

							// タイマーを終了します。
							::KillTimer(hwnd, w_param);

							break;
						}
					}

					break;
				}
			case WM_SIZE:
				{
					MY_TRACE_FUNC("{/}, {/hex}, {/hex}", my::message_to_string(message), w_param, l_param);

					// ダイアログのレイアウトを更新します。
					recalc_layout();

					break;
				}
			case WM_NOTIFY:
				{
//					MY_TRACE_FUNC("{/}, {/hex}, {/hex}", my::message_to_string(message), w_param, l_param);

					auto nmh = (NMHDR*)l_param;

					switch (nmh->code)
					{
					case LVN_COLUMNCLICK:
						{
							MY_TRACE_FUNC("WM_NOTIFY, LVN_COLUMNCLICK");

							// リストビューではない場合は何もしません。
							if (nmh->hwndFrom != listview) break;

							// カラムがクリックされたときの処理です。

							auto nm = (NM_LISTVIEW*)l_param;

							// クリックされたカラムを基準にリストビューをソートします。
							// 同時に、クリックされたカラムのテキストを更新します。
							sort_items(nm->iSubItem);

							break;
						}
					case NM_CLICK:
						{
							MY_TRACE_FUNC("WM_NOTIFY, NM_CLICK");

							// リストビューではない場合は何もしません。
							if (nmh->hwndFrom != listview) break;

							// リストビューがクリックされたときの処理です。

							auto nm = (NMITEMACTIVATE*)l_param;

							// クリックしたオブジェクトをフォーカスオブジェクトにします。
							app->focus_to(nm->iItem, nm->iSubItem);

							break;
						}
					case NM_DBLCLK:
						{
							MY_TRACE_FUNC("WM_NOTIFY, NM_DBLCLK");

							// リストビューではない場合は何もしません。
							if (nmh->hwndFrom != listview) break;

							// リストビューがダブルクリックされたときの処理です。

							auto nm = (NM_LISTVIEW*)l_param;

							// ダブルクリックしたオブジェクトが
							// 中央に表示されるようにタイムラインをスクロールします。
							app->scroll_to(nm->iItem, nm->iSubItem);

							break;
						}
					case NM_RCLICK:
						{
							MY_TRACE_FUNC("WM_NOTIFY, NM_RCLICK");

							// リストビューではない場合は何もしません。
							if (nmh->hwndFrom != listview) break;

							// リストビューが右クリックされたときの処理です。

							auto nm = (NMITEMACTIVATE*)l_param;

							// クリックされた行用のコンテキストメニューを表示します。
							show_context_menu(nm->iItem, nm->iSubItem);

							// 1を返さないとWM_CONTEXTMENUが呼ばれてしまいます。
							return 1;
						}
					case NM_CUSTOMDRAW:
						{
//							MY_TRACE_FUNC("WM_NOTIFY, NM_CUSTOMDRAW");

							// リストビューではない場合は何もしません。
							if (nmh->hwndFrom != listview) break;

							auto nm = (NMLVCUSTOMDRAW*)l_param;

							switch (nm->nmcd.dwDrawStage)
							{
							case CDDS_PREPAINT:
								{
									return CDRF_NOTIFYITEMDRAW;
								}
							case CDDS_ITEMPREPAINT:
								{
									nm->clrText = CLR_DEFAULT;
									nm->clrTextBk = CLR_DEFAULT;

									return CDRF_NOTIFYPOSTPAINT;
								}
							case CDDS_ITEMPOSTPAINT:
								{
									auto dc = nm->nmcd.hdc;
									auto rc = nm->nmcd.rc;

									// 項目のインデックスを取得します。
									auto item_index = nm->nmcd.dwItemSpec;
									if ((size_t)item_index >= finder.nodes.size()) break;

									// ノードを取得します。
									const auto& node = finder.nodes[item_index];

									// レイヤーの色を描画します。
									auto layer_rc = RECT {};
									ListView_GetSubItemRect(listview, item_index, list.k_column.k_layer, LVIR_ICON, &layer_rc);
									layer_rc.left -= 4;
									color_manager.fill_rect(dc, &layer_rc, node->layer_color);

									// オブジェクトの色を描画します。
									auto name_rc = RECT {};
									ListView_GetSubItemRect(listview, item_index, list.k_column.k_name, LVIR_ICON, &name_rc);
									name_rc.right += 2;
									color_manager.draw_rect(dc, &name_rc, node->color_index);

									break;
								}
							}

							break;
						}
					case LVN_GETDISPINFO:
						{
//							MY_TRACE_FUNC("WM_NOTIFY, LVN_GETDISPINFO");

							// リストビューではない場合は何もしません。
							if (nmh->hwndFrom != listview) break;

							// リストビュー項目の文字列をセットするときの処理です。

							auto nm = (NMLVDISPINFO*)l_param;

							// 項目のインデックスを取得します。
							auto item_index = nm->item.iItem;
							auto sub_item_index = nm->item.iSubItem;

							// 項目のインデックスが範囲外の場合は何もしません。
							if ((size_t)item_index >= finder.nodes.size()) break;
							if ((size_t)sub_item_index >= (size_t)list.k_column.k_max_size) break;

							// 項目テキストを設定する必要がある場合は
							if (nm->item.mask & LVIF_TEXT)
							{
								// 項目に対応するノードを取得します。
								const auto& node = finder.nodes[item_index];

								// 列に対応する文字列をセットします。
								nm->item.pszText = (LPWSTR)node->get_text(sub_item_index).data();
							}

							// 項目テキストを設定する必要がある場合は
							if (nm->item.mask & LVIF_IMAGE)
							{
								// イメージを持つ列の場合は
								if (list.k_column.k_datas[sub_item_index].flag_has_image)
								{
									// ダミーのイメージをセットします。
									nm->item.iImage = 0;
								}
							}

							break;
						}
					case HDN_ENDFILTEREDIT:
						{
							MY_TRACE_FUNC("WM_NOTIFY, HDN_ENDFILTEREDIT");

							auto nm = (NMHEADER*)l_param;

							// リストビューのヘッダーコントロールではない場合は何もしません。
							if (header != nmh->hwndFrom) break;

							// ここでどのフィルタが作動したかどうかは識別することが不可能です。
							// よって、ヘッダーコントロールの全てのフィルタをチェックする必要があります。
							header.from_ui();

							// 絞り込みを適用します。
							app->apply_finder();

							break;
						}
					case HDN_FILTERBTNCLICK:
						{
							MY_TRACE_FUNC("WM_NOTIFY, HDN_FILTERBTNCLICK");

							// ヘッダーコントロールのフィルタボタンをクリックしたときの処理です。

							auto nm = (NMHDFILTERBTNCLICK*)l_param;

							// リストビューのヘッダーコントロールではない場合は何もしません。
							if (header != nmh->hwndFrom) break;

							// 絞り込みメニューを表示します。
							header.show_filter_menu(nm->iItem, &nm->rc);

							break;
						}
					}

					break;
				}
			}

			return __super::on_wnd_proc(hwnd, message, w_param, l_param);
		}
	} main_window;
}
