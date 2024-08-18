#pragma once

//
// このクラスはフィルタのコンボボックスのウィンドウプロシージャをフックします。
//
inline struct CreateComboBox : Addin
{
	//
	// コンストラクタです。
	//
	CreateComboBox()
	{
		addins.emplace_back(this);
	}

	//
	// 処理中のオブジェクトです。
	//
	inline static ObjectPtr object = {};

	//
	// 処理中のフィルタです。
	//
	inline static FilterPtr filter = {};

	//
	// 処理中のアイテムのインデックスです。
	//
	inline static int32_t item_index = 0;

	//
	// デフォルト処理をスキップします。
	//
	inline static BOOL skip_default = FALSE;

	//
	// オリジナルのウィンドウプロシージャです。
	//
	inline static WNDPROC orig_combobox_wnd_proc = nullptr;

	//
	// オリジナルのウィンドウプロシージャを実行します。
	//
	inline static LRESULT WINAPI do_default(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
	{
		return ::CallWindowProcA(orig_combobox_wnd_proc, hwnd, message, wparam, lparam);
	}

	//
	// オリジナルのウィンドウプロシージャを実行します。
	// ただし、再描画しません。
	//
	inline static LRESULT WINAPI do_default_no_redraw(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
	{
		if (hive.fp->check[1])
		{
			::DefWindowProcA(hwnd, WM_SETREDRAW, FALSE, 0);
			auto result = ::CallWindowProcA(orig_combobox_wnd_proc, hwnd, message, wparam, lparam);
			::DefWindowProcA(hwnd, WM_SETREDRAW, TRUE, 0);
			return result;
		}
		else
		{
			return ::CallWindowProcA(orig_combobox_wnd_proc, hwnd, message, wparam, lparam);
		}
	}

	//
	// ウィンドウプロシージャです。
	//
	inline static LRESULT CALLBACK combobox_wnd_proc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
	{
		switch (message)
		{
		case CB_RESETCONTENT:
			{
				output_debug_string("{:#010x}, CB_RESETCONTENT", (uint32_t)hwnd);

				dump_filter_controls();

				// まず、変数をリセットします。
				object = {};
				filter = {};
				item_index = 0;
				skip_default = FALSE;

				if (hive.fp->check[0])
				{
					// カレントオブジェクトを取得します。
					if (!object.init(exin::get_current_object_index()))
						return do_default_no_redraw(hwnd, message, wparam, lparam);

					// アニメーション効果を取得します。
					filter = object.get_animation_effect(hwnd);

					// アニメーション効果の場合は
					if (filter.get())
					{
						output_debug_string("{:#010x} => アニメーション効果", (uint32_t)hwnd);

						// デフォルト処理をスキップして
						// コンボボックスのコンテンツを維持します。
						return CB_OKAY;
					}
					// アニメーション効果以外の場合は
					else
					{
						output_debug_string("{:#010x} => アニメーション効果以外", (uint32_t)hwnd);

						// コンボボックスのカテゴリをリセットします。
						set_category_id(hwnd, 0);

						// デフォルト処理を実行します。
						return do_default_no_redraw(hwnd, message, wparam, lparam);
					}
				}
				else
				{
					// デフォルト処理を実行します。
					return do_default_no_redraw(hwnd, message, wparam, lparam);
				}
			}
		case CB_INSERTSTRING:
			{
				// アニメーション効果の場合は
				if (filter.get())
				{
					// 処理中のアイテムのインデックスを取得します。
					auto result = item_index++;

					// フラグが立っている場合はデフォルト処理をスキップします。
					if (skip_default) return result;

					output_debug_string("{:#010x}, CB_INSERTSTRING, {}, {:#010x}, {}", (uint32_t)hwnd, result, lparam, (LPCSTR)lparam);

					// 先頭アイテムの場合は
					if (result == 0)
					{
						// フィルタのカテゴリを取得します。
						auto filter_category_id = animation_effect_manager.get_category_id(object.get(), filter.index());
						output_debug_string("フィルタカテゴリ => {}", filter_category_id);

						// フィルタのカテゴリが設定されていて、
						// なおかつ最初のアイテム文字列がアニメーション効果名バッファの場合は
						if (filter_category_id > 2 && (LPCSTR)lparam == animation_effect_manager.table)
						{
							output_debug_string("あとで再構築されるのでデフォルト処理をスキップします");

							// このコンボボックスをあとで再構築されるので
							// 今回はデフォルト処理をスキップするようにします。
							skip_default = TRUE;
							return result;
						}

						// コンボボックスのカテゴリを取得します。
						auto cached_category_id = get_category_id(hwnd);
						output_debug_string("キャッシュカテゴリ => {}", cached_category_id);

						// キャッシュされたカテゴリが存在する場合は
						if (cached_category_id)
						{
							// カテゴリが同一の場合は
							if (filter_category_id == cached_category_id)
							{
								output_debug_string("キャッシュが存在するのでデフォルト処理をスキップします");

								// コンボボックスを再構築する必要がないので
								// デフォルト処理をスキップするようにします。
								skip_default = TRUE;
								return result;
							}
						}

						// スキップしていたコンテンツリセット処理をここで実行します。
						do_default_no_redraw(hwnd, CB_RESETCONTENT, 0, 0);

						// コンボボックスにカテゴリを関連付けます。
						set_category_id(hwnd, filter_category_id);
					}
				}

				return do_default_no_redraw(hwnd, message, wparam, lparam);
			}
		}

		return do_default(hwnd, message, wparam, lparam);
	}

	//
	// この関数は::CreateWindowExA()の呼び出しと置き換えられるフック関数です。
	//
	inline static HWND WINAPI hook_proc(
		decltype(&::CreateWindowExA) create_window,
		LPCSTR window_name, DWORD style, int x, int y, int w, int h,
		HWND parent, HMENU menu, HINSTANCE instance, LPVOID param)
	{
		auto hwnd = create_window(0, "COMBOBOX", window_name, style, x, y, w, h, parent, menu, instance, param);
		orig_combobox_wnd_proc = (WNDPROC)::SetWindowLongPtrA(hwnd, GWLP_WNDPROC, (LONG_PTR)combobox_wnd_proc);
		return hwnd;
	}

	//
	// 初期化処理を実行します。
	// 拡張編集内のフィルタのコンボボックスを作成する部分のコードを書き換えます。
	//
	virtual BOOL func_init(AviUtl::FilterPlugin* fp) override
	{
/*
90            NOP
90            NOP
90            NOP
53            PUSH EBX
E8 func       CALL func
合計 9

$+2F1A9   |.  68 48480A10    |PUSH OFFSET 100A4848                    ; ASCII "COMBOBOX"
$+2F1AE   |.  6A 00          |PUSH 0
$+2F1B0   |.  FFD3           |CALL EBX
合計 9
*/
#pragma pack(push, 1)
		struct Code {
			uint8_t nop_1 = 0x90;
			uint8_t nop_2 = 0x90;
			uint8_t nop_3 = 0x90;
			uint8_t push_ebx = 0x53;
			uint8_t call = 0xe8;
			uint32_t hook_proc;
		} code;
		assert(sizeof(Code) == 9);
#pragma pack(pop)

		uint32_t address = exin::exedit + 0x0002F1A9;
		code.hook_proc = (uint32_t)hook_proc - (address + sizeof(code));
		::WriteProcessMemory(::GetCurrentProcess(),
			(void*)address, &code, sizeof(Code), nullptr);

		return TRUE;
	}

	//
	// 後始末処理を実行します。
	//
	virtual BOOL func_exit(AviUtl::FilterPlugin* fp) override
	{
		return TRUE;
	}
} create_combobox;
