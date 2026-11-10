#pragma once

namespace apn::timeline_object_lister
{
/*
; 映像メディアオブジェクトの色
ObjectVideo=3040e0,2030c0
; 映像メディアオブジェクトの色(選択時)
ObjectVideoSelect=5868ff
; 音声メディアオブジェクトの色
ObjectAudio=d04030,b83020
; 音声メディアオブジェクトの色(選択時)
ObjectAudioSelect=e06050
; 制御オブジェクトの色
ObjectControl=30b0c0,2090a0
; 制御オブジェクトの色(選択時)
ObjectControlSelect=50c0d0
; 映像フィルタオブジェクトの色
ObjectVideoFilter=30b030,209020
; 映像フィルタオブジェクトの色(選択時)
ObjectVideoFilterSelect=50c050
; 音声フィルタオブジェクトの色
ObjectAudioFilter=b8b030,989020
; 音声フィルタオブジェクトの色(選択時)
ObjectAudioFilterSelect=c8c050
; 映像フィルタ効果オブジェクトの色
ObjectVideoEffect=30b0c0,2090a0
; 映像フィルタ効果オブジェクトの色(選択時)
ObjectVideoEffectSelect=50c0d0
; 音声フィルタ効果オブジェクトの色
ObjectAudioEffect=30b0c0,2090a0
; 音声フィルタ効果オブジェクトの色(選択時)
ObjectAudioEffectSelect=50c0d0

	static constexpr int EFFECT_TYPE_FILTER		= 1;	// フィルタ効果
	static constexpr int EFFECT_TYPE_INPUT		= 2;	// メディア入力
	static constexpr int EFFECT_TYPE_TRANSITION = 3;	// シーンチェンジ
	static constexpr int EFFECT_TYPE_CONTROL	= 4;	// オブジェクト制御
	static constexpr int EFFECT_TYPE_OUTPUT		= 5;	// メディア出力
	// エフェクトフラグ ※今後追加される可能性があります
	static constexpr int EFFECT_FLAG_VIDEO		= 1;	// 画像をサポート
	static constexpr int EFFECT_FLAG_AUDIO		= 2;	// 音声をサポート
	static constexpr int EFFECT_FLAG_FILTER		= 4;	// フィルタオブジェクトをサポート
	static constexpr int EFFECT_FLAG_CAMERA		= 8;	// カメラ効果をサポート

// エフェクト名					タイプ		フラグ
フィルタオブジェクト			0x00000004	0x00000003
	EFFECT_TYPE_CONTROL
	EFFECT_FLAG_VIDEO | EFFECT_FLAG_AUDIO
部分フィルタ					0x00000004	0x00000001
	EFFECT_TYPE_CONTROL
	EFFECT_FLAG_VIDEO
グラデーション					0x00000001	0x00000005
ぼかし			0x00000001	0x00000005
	EFFECT_TYPE_FILTER
	EFFECT_FLAG_VIDEO | EFFECT_FLAG_FILTER

2個目のエフェクトがEFFECT_TYPE_FILTER(フィルタ効果)の場合、
先頭のエフェクト名が"フィルタオブジェクト"の場合はフィルタオブジェクト。
そうでない場合はフィルタ効果オブジェクト。
つまり、2個目のエフェクトのEFFECT_FLAG_FILTERをチェックしても意味がない。

ただし、"フィルタオブジェクト"だけでは画像なのか音声なのかは判別できない。
2個目のエフェクトのEFFECT_FLAG_VIDEOとEFFECT_FLAG_AUDIOをチェックしなければならない。
更に、"フィルタオブジェクト"はEFFECT_TYPE_CONTROLなので、制御系のオブジェクトと判別できない。

よって、先頭のエフェクト名だけでは配色を特定することはできない。
2個目のエフェクト名だけで配色を特定することもできない。

先頭2つのエフェクト名が必要。
更に、エフェクトのタイプ・フラグが必要。
しかし、エフェクト名からタイプ・フラグを取得することはできない。

エフェクトのタイプ・フラグは事前に列挙しなければならない。
ただし、RegisterPlugin()では列挙できない。
他のイベントハンドラの初回に列挙しなければならない。

スタイルファイルには"映像フィルタオブジェクトの色"と書いてあるが、
これは"フィルタオブジェクト"が付いているオブジェクトの色ではない。

"フィルタオブジェクト"が付いていないオブジェクトは"フィルタオブジェクトの色"
"フィルタオブジェクト"が付いているオブジェクトが"フィルタ効果オブジェクトの色"

"フィルタ効果の追加"からオブジェクトを選択すると、
"フィルタオブジェクト"が付いていないオブジェクトが生成される。

"フィルタオブジェクト"(アプリの表記)➡"フィルタオブジェクト"(API)➡"フィルタ効果オブジェクトの色"(スタイルファイル)➡画像は緑(30b030)・音声は黄色(b8b030)
"フィルタ効果"(アプリの表記)➡非"フィルタオブジェクト"(API)➡"フィルタオブジェクトの色"(スタイルファイル)➡画像・音声共に水色(30b0c0)
画像の色は、RGBの内、Bのみの1byteしか違わないので、見間違えないように注意が必要。
フィルタ効果が画像も音声も同じ色なのはバグではなく、デフォルトのスタイルがそうなっているから。
*/
	//
	// このクラスは配色を管理します。
	//
	inline struct color_manager_t
	{
		//
		// タイマーのIDです。現在は使用されていません。
		//
		inline static constexpr struct index_t {
			inline static constexpr auto k_video = 0;
			inline static constexpr auto k_audio = 1;
			inline static constexpr auto k_video_filter = 2;
			inline static constexpr auto k_audio_filter = 3;
			inline static constexpr auto k_video_filter_object = 4;
			inline static constexpr auto k_audio_filter_object = 5;
			inline static constexpr auto k_control = 6;
			inline static constexpr auto k_max_size = 7;
		} k_index;

		//
		// このクラスはタイムラインオブジェクトの配色です。
		//
		struct color_t {
			COLORREF start, end;
		};

		//
		// 配色のテーブルです。
		//
		color_t colors[k_index.k_max_size] = {};

		//
		// 初期化後初期化処理を実行します。
		//
		BOOL post_init()
		{
			MY_TRACE_FUNC("");

			// コンフィグハンドルが無効の場合は何もしません。
			if (!tr.config) return FALSE;

			//
			// この関数は、指定されたキーの配色を返します。
			//
			const auto get_color = [](LPCSTR key)
			{
				//
				// この関数は、aviutl2のカラーコードをCOLORREFに変換して返します。
				//
				const auto to_win32_color = [](int color_code)
				{
					auto a = (color_code & 0xff000000) >> 24;
					auto r = (color_code & 0x00ff0000) >> 16;
					auto g = (color_code & 0x0000ff00) >> 8;
					auto b = (color_code & 0x000000ff) >> 0;

					return RGB(r, g, b);
				};

				return color_t {
					to_win32_color(tr.config->get_color_code_index(tr.config, key, 0)),
					to_win32_color(tr.config->get_color_code_index(tr.config, key, 1)),
				};
			};

			// フィルタオブジェクトとフィルタ効果の色を
			// 逆に指定しているように見えますが、これで正しいです。
			colors[k_index.k_video] = get_color("ObjectVideo");
			colors[k_index.k_audio] = get_color("ObjectAudio");
			colors[k_index.k_video_filter_object] = get_color("ObjectVideoFilter");
			colors[k_index.k_audio_filter_object] = get_color("ObjectAudioFilter");
			colors[k_index.k_video_filter] = get_color("ObjectVideoEffect");
			colors[k_index.k_audio_filter] = get_color("ObjectAudioEffect");
			colors[k_index.k_control] = get_color("ObjectControl");

			return TRUE;
		}

		//
		// 2色のグラデーションを描画する関数です
		// http://iooiau.net/tips/web20back.html
		//
		inline static BOOL TwoColorsGradient(
			HDC hdc,            // 描画先のデバイスコンテキスト・ハンドルです
			const RECT *pRect,  // 描画する範囲の矩形です
			COLORREF Color1,    // 描画する一つ目の色です
			COLORREF Color2,    // 描画する二つ目の色です
			BOOL fHorizontal    // 水平のグラデーションを描画する場合は TRUE にします
		)
		{
			TRIVERTEX vert[2];
			GRADIENT_RECT rect = {0, 1};

			// 描画範囲と色を設定します
			vert[0].x = pRect->left;
			vert[0].y = pRect->top;
			vert[0].Red   = GetRValue(Color1) << 8;
			vert[0].Green = GetGValue(Color1) << 8;
			vert[0].Blue  = GetBValue(Color1) << 8;
			vert[0].Alpha = 0;
			vert[1].x = pRect->right;
			vert[1].y = pRect->bottom;
			vert[1].Red   = GetRValue(Color2) << 8;
			vert[1].Green = GetGValue(Color2) << 8;
			vert[1].Blue  = GetBValue(Color2) << 8;
			vert[1].Alpha = 0;
			return ::GdiGradientFill(hdc, vert, 2, &rect, 1,
				fHorizontal ? GRADIENT_FILL_RECT_H : GRADIENT_FILL_RECT_V);
		}

		//
		// 指定されたエフェクト名に対応する配色で矩形を描画します。
		//
		BOOL draw_rect(HDC dc, LPCRECT rc, int color_index)
		{
			// 配色を取得します。
			const auto& color = colors[color_index];

			// 矩形を配色で塗りつぶします。
			return TwoColorsGradient(dc, rc, color.start, color.end, TRUE);
		}

		//
		// 指定された色で矩形を描画します。
		//
		BOOL fill_rect(HDC dc, LPCRECT rc, COLORREF fill_color)
		{
			if (fill_color == CLR_DEFAULT)
				return ::FillRect(dc, rc, ::GetSysColorBrush(COLOR_WINDOW));

			my::gdi<HBRUSH>::unique_ptr brush(::CreateSolidBrush(fill_color));
			my::gdi<HBRUSH>::selector brush_selector(dc, brush.get());

			return ::PatBlt(dc, rc->left, rc->top, my::get_width(*rc), my::get_height(*rc), PATCOPY);
		}

		//
		// 指定されたエフェクト名に対応する配色のインデックスを返します。
		//
		int get_index(LPCWSTR effect_name, BOOL flag_is_filter_object)
		{
			// エフェクト情報を取得します。
			auto effect_info = effect_manager.get_effect_info(effect_name);
			auto type = effect_info.type;
			auto flag = effect_info.flag;

			// フィルタオブジェクトかどうか判別するには、専用のフラグが必要です。
			if (flag_is_filter_object)
				return (flag & EDIT_HANDLE::EFFECT_FLAG_VIDEO) ? k_index.k_video_filter_object : k_index.k_audio_filter_object;

			switch (type)
			{
			default:
			case EDIT_HANDLE::EFFECT_TYPE_TRANSITION:
			case EDIT_HANDLE::EFFECT_TYPE_OUTPUT:
			case EDIT_HANDLE::EFFECT_TYPE_FILTER:
				{
//					if (flag & EDIT_HANDLE::EFFECT_FLAG_FILTER)
						return (flag & EDIT_HANDLE::EFFECT_FLAG_VIDEO) ? k_index.k_video_filter : k_index.k_audio_filter;
//					else
//						return (flag & EDIT_HANDLE::EFFECT_FLAG_VIDEO) ? k_index.k_video_filter_object : k_index.k_audio_filter_object;
				}
			case EDIT_HANDLE::EFFECT_TYPE_INPUT:
				{
					return (flag & EDIT_HANDLE::EFFECT_FLAG_VIDEO) ? k_index.k_video : k_index.k_audio;
				}
			case EDIT_HANDLE::EFFECT_TYPE_CONTROL:
				{
					return k_index.k_control;
				}
			}
		}
	} color_manager;
}
