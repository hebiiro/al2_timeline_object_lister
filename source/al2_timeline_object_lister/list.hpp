#pragma once

namespace apn::timeline_object_lister
{
	//
	// このクラスはリストです。
	//
	inline struct list_t
	{
		//
		// このクラスはリストの列です。
		//
		inline static constexpr struct column_t {
			inline static constexpr int k_layer = 0;
			inline static constexpr int k_status = 1;
			inline static constexpr int k_name = 2;
			inline static constexpr int k_info = 3;
			inline static constexpr int k_start = 4;
			inline static constexpr int k_range = 5;
			inline static constexpr int k_max_size = 6;
			inline static constexpr my::Label labels[] = {
				{ k_layer, L"layer" },
				{ k_status, L"status" },
				{ k_name, L"name" },
				{ k_info, L"info" },
				{ k_start, L"start" },
				{ k_range, L"range" },
			};
			inline static constexpr struct data_t {
				LPCWSTR text; int width; BOOL flag_editable; BOOL flag_number; BOOL flag_has_image;
			} k_datas[] = {
				{ L"レイヤー", 100, FALSE, TRUE, TRUE },
				{ L"ステータス", 100, FALSE, TRUE, FALSE },
				{ L"名前", 100, FALSE, FALSE, TRUE },
				{ L"フィルタ", 200, FALSE, TRUE, FALSE },
				{ L"位置", 100, TRUE, TRUE, FALSE  },
				{ L"長さ", 100, TRUE, TRUE, FALSE },
			};
		} k_column;

		//
		// このクラスはリストノードです。
		//
		struct node_t
		{
			int orig_index = -1;
			OBJECT_LAYER_FRAME object_layer_frame;
			COLORREF layer_color = CLR_DEFAULT;
			int color_index = 0;
			std::wstring values[k_column.k_max_size];

			//
			// 指定された列に対応する文字列を返します。
			//
			inline const std::wstring& get_text(int col_index) const { return values[col_index]; }

			//
			// 指定された列に対応する文字列を変更します。
			//
			inline void set_text(int col_index, const std::wstring& value) { values[col_index] = value; }
		};

		//
		// リストノードのコレクションです。
		//
		std::vector<std::shared_ptr<node_t>> nodes;

		//
		// ノードを新規作成して返します。
		//
		auto& create_node(const OBJECT_LAYER_FRAME& object_layer_frame, COLORREF layer_color, int color_index)
		{
			return nodes.emplace_back(std::make_shared<node_t>(
				(int)nodes.size(), object_layer_frame, layer_color, color_index));
		}
	} list;
}
