#pragma once

namespace apn::timeline_object_lister
{
	//
	// このクラスはリストノードを絞り込みます。
	//
	struct finder_t
	{
		//
		// 絞り込み用テキストのコレクションです。
		//
		std::wstring texts[list.k_column.k_max_size];

		//
		// リストノードの絞り込み済みコレクションです。
		//
		std::vector<std::shared_ptr<list_t::node_t>> nodes;

		//
		// 指定されたリストノードが絞り込み対象の場合はTRUEを返します。
		//
		BOOL not_found(const std::shared_ptr<list_t::node_t>& node)
		{
			// 絞り込み用テキストを走査します。
			for (auto i = size_t {}; i < std::size(texts); i++)
			{
				// 絞り込み用テキストが無効の場合は何もしません。
				if (texts[i].empty()) continue;

				// 指定されたリストノード内に
				// 絞り込み用テキストが見つからなかった場合はTRUEを返します。
				if (node->get_text((int32_t)i).find(texts[i]) == texts[i].npos) return TRUE;
			}

			// 指定されたリストノードは絞り込み対象外なのでFALSEを返します。
			return FALSE;
		}

		//
		// リストノードを絞り込みます。
		// (絞り込み済みコレクションを構築します)
		//
		BOOL apply()
		{
			// 一旦絞り込み済みコレクションをリセットします。
			nodes.clear();

			// リストノードを走査します。
			for (const auto& node : list.nodes)
			{
				// リストノードが絞り込み対象の場合は何もしません。
				if (not_found(node)) continue;

				// このリストノードを絞り込み済みコレクションに追加します。
				nodes.emplace_back(node);
			}

			return TRUE;
		}

		//
		// 絞り込み用テキストをリセットします。
		//
		BOOL reset()
		{
			// 絞り込み用テキストを走査します。
			for (auto i = size_t {}; i < std::size(texts); i++)
			{
				// 絞り込み用テキストをリセットします。
				texts[i].clear();
			}

			return TRUE;
		}
	} finder;
}
