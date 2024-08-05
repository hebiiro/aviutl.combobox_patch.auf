#include <string>
#include <vector>
#include <unordered_map>
#include <format>
#include <filesystem>
#include "aviutl.hpp"
#include "exedit.hpp"
#include "detours.4.0.1/detours.h"
#pragma comment(lib, "detours.4.0.1/detours.lib")

constexpr auto plugin_name = "コンボボックスパッチ";
constexpr auto plugin_information = "コンボボックスパッチ r1 by 蛇色 / originator 狐紺くろ(Tsut-ps) / X";

//
// コンボボックスを操作するための機能群です。
//
namespace cb
{
	std::size_t get_count(HWND hwnd)
	{
		return (std::size_t)::SendMessageA(hwnd, CB_GETCOUNT, 0, 0);
	}

	std::string get_text(HWND hwnd, std::size_t index)
	{
		auto text_length = ::SendMessageA(hwnd, CB_GETLBTEXTLEN, index, 0);
		if (text_length == CB_ERR) return {};
		std::string text(text_length, '\0');
		::SendMessageA(hwnd, CB_GETLBTEXT, index, (LPARAM)text.data());
		return text;
	}
}

//
// 拡張編集にアクセスするための機能群です。
//
namespace exin
{
	DWORD exedit = 0;
	std::vector<std::string> animation_names;

	struct {
		ExEdit::Object** object_table = nullptr;
		ExEdit::Object** sorted_object_table = nullptr;
		ExEdit::Filter** filter_table = nullptr;
		int32_t* current_scene_index = nullptr;
		int32_t* current_object_index = nullptr;
		int32_t* current_filter_index = nullptr;
		int32_t* object_count = nullptr;
		int32_t* sorted_object_count = nullptr;
		uint8_t** exdata_table = nullptr;
		const char* animation_names = nullptr;
		HWND* filter_checkbox_table = nullptr;
	} variable;

	struct {
		BOOL (CDECL* update_controls)(int32_t object_index) = nullptr;
	} function;

	void init()
	{
		exedit = (decltype(exedit))::GetModuleHandleW(L"exedit.auf");
		variable.object_table = (decltype(variable.object_table))(exedit + 0x001E0FA4);
		variable.sorted_object_table = (decltype(variable.sorted_object_table))(exedit + 0x00168FA8);
		variable.filter_table = (decltype(variable.filter_table))(exedit + 0x00187C98);
		variable.current_object_index = (decltype(variable.current_object_index))(exedit + 0x00177A10);
		variable.exdata_table = (decltype(variable.exdata_table))(exedit + 0x001E0FA8);
		variable.animation_names = (decltype(variable.animation_names))(exedit + 0x000C1F08);
		variable.filter_checkbox_table = (decltype(variable.filter_checkbox_table))(exedit + 0x0014D368);
		function.update_controls = (decltype(function.update_controls))(exedit + 0x000305E0);

		{
			auto animation_name = variable.animation_names;
			while (animation_name[0])
			{
				auto length = strlen(animation_name);
				animation_names.emplace_back(animation_name, length);
				animation_name += length + 1;
			}
		}
	}

	ExEdit::Object* get_object(int32_t object_index) { return *variable.object_table + object_index; }
	ExEdit::Object* get_sorted_object(int32_t object_index) { return variable.sorted_object_table[object_index]; }
	ExEdit::Filter* get_filter(int32_t filter_id) { return variable.filter_table[filter_id]; }
	int32_t get_current_object_index() { return *variable.current_object_index; }

	uint32_t get_exdata_offset(ExEdit::Object* object, int32_t filter_index)
	{
		return object->exdata_offset + object->filter_param[filter_index].exdata_offset;
	}

	uint8_t* get_exdata(ExEdit::Object* object, int32_t filter_index)
	{
		uint8_t* exdata_table = *variable.exdata_table;
		auto offset = get_exdata_offset(object, filter_index);
		return exdata_table + offset + 0x0004;
	}

	const char* get_animation_name(size_t animation_type)
	{
		if (animation_type < animation_names.size())
			return animation_names[animation_type].c_str();
		else
			return "invalid_animation_type";
	}

	HWND get_filter_checkbox(int32_t filter_index) { return variable.filter_checkbox_table[filter_index]; }

	BOOL update_controls(int32_t object_index) { return function.update_controls(object_index); }
}

//
// フィルタプラグインです。
//
AviUtl::FilterPlugin* fp = nullptr;

//
// カレントのアイテム名です。
//
char* current_item_names = nullptr;

namespace deploy_filter_controls
{
	uint32_t (CDECL* orig_proc)(uint32_t u1, int32_t object_index, int32_t filter_index, uint32_t u4, uint32_t u5, uint32_t u6, uint32_t u7) = nullptr;
	uint32_t CDECL hook_proc(uint32_t u1, int32_t object_index, int32_t filter_index, uint32_t u4, uint32_t u5, uint32_t u6, uint32_t u7)
	{
		try
		{
			auto object = exin::get_object(object_index);
			if (!object) throw L"オブジェクトが無効です";

			if (object->index_midpt_leader >= 0)
			{
				object_index = object->index_midpt_leader;
				object = exin::get_object(object_index);
				if (!object) throw L"中間点リーダーが無効です";
			}

			auto filter_id = object->filter_param[filter_index].id;
			if (filter_id < 0) throw L"フィルタIDが無効です";

			auto filter = exin::get_filter(filter_id);
			if (!filter) throw L"フィルタが無効です";

//			if (::lstrcmpA(filter->name, "アニメーション効果") == 0)
			{
				// 一時的にカレントのアイテム名をセットしてからデフォルト処理を実行します。
				// デフォルト処理の中で呼ばれる::SendMessageA()をフックします。
				// ここでセットしたcurrent_item_namesはフック関数内で使用されます。
				current_item_names = filter->check_name ? filter->check_name[0] : nullptr;
				auto result = orig_proc(u1, object_index, filter_index, u4, u5, u6, u7);
				current_item_names = nullptr;
				return result;
			}
		}
		catch (...)
		{
		}

		return orig_proc(u1, object_index, filter_index, u4, u5, u6, u7);
	}

	BOOL init()
	{
		orig_proc = decltype(orig_proc)(exin::exedit + 0x2FA90);
		DetourAttach(&(PVOID&)orig_proc, hook_proc);

		return TRUE;
	}

	BOOL exit()
	{
		DetourDetach(&(PVOID&)orig_proc, hook_proc);

		return TRUE;
	}
}

//
// アイテム名のリストからアイテム名を取り出して返します。
// さらにアイテム名のリストを次に進めます。
//
std::string get_item_name(LPCSTR& item_names)
{
	LPCSTR begin = item_names;
	LPCSTR end = nullptr;

	while (*item_names)
	{
		if (*item_names == 0x01)
			end = item_names;

		item_names++;
	}

	if (!end) end = item_names;

	item_names++;

	return { begin, end };
}

//
// コンボボックスがアイテム名のリストと一致する場合はTRUEを返します。
//
BOOL is_equal(HWND hwnd, LPCSTR item_names)
{
	auto cb_count = cb::get_count(hwnd);
	if (cb_count == 0) return FALSE;

	for (decltype(cb_count) i = 0; i < cb_count; i++)
	{
		auto cb_text = cb::get_text(hwnd, i);
		if (cb_text.empty()) return FALSE;

		auto item_name = get_item_name(item_names);
		if (cb_text != item_name) return FALSE;
	}

	return TRUE;
}

//
// 一部の::SendMessageA()の呼び出しをフックします。
//
LRESULT WINAPI hook_SendMessageA(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
	static BOOL skip_default = FALSE;

	switch (message)
	{
	case CB_RESETCONTENT:
		{
			// 『コンボボックスを再描画しない』が有効の場合は再描画を停止します。
			if (fp->check[1])
				::SendMessageA(hwnd, WM_SETREDRAW, FALSE, 0);

			// 『コンボボックスを再構築しない』が有効の場合は
			if (fp->check[0] && current_item_names)
			{
				// アイテムリストが同じ場合は
				if (is_equal(hwnd, current_item_names))
				{
					// コンボボックスを再構築する必要がないので
					// デフォルト処理をスキップするようにします。
					skip_default = TRUE;
					return CB_OKAY;
				}
			}

			// デフォルト処理を実行するようにします。
			skip_default = FALSE;

			break;
		}
	case CB_INSERTSTRING:
		{
			// 『コンボボックスを再構築しない』が有効の場合は
			if (fp->check[0] && current_item_names)
			{
				if (skip_default) return CB_OKAY;
			}

			break;
		}
	case CB_SETDROPPEDWIDTH:
		{
			// 『コンボボックスを再描画しない』が有効の場合は再描画を再開します。
			if (fp->check[1])
				::SendMessageA(hwnd, WM_SETREDRAW, TRUE, 0);

			break;
		}
	}

	return ::SendMessageA(hwnd, message, wparam, lparam);
}

//
// 書き換えるアドレスです。
//
struct Address
{
	uint32_t relative_address;
	decltype(&::SendMessageA)* ptr_ptr_orig_SendMessageA;
	decltype(&::SendMessageA) ptr_hook_SendMessageA;
	decltype(&::SendMessageA)* ptr_ptr_hook_SendMessageA;
} collection[] =
{
	{ 0x2FF5B + 2, nullptr, &hook_SendMessageA, &collection[0].ptr_hook_SendMessageA },
	{ 0x2FFF7 + 2, nullptr, &hook_SendMessageA, &collection[1].ptr_hook_SendMessageA },
	{ 0x30099 + 2, nullptr, &hook_SendMessageA, &collection[2].ptr_hook_SendMessageA },
};

//
// 初期化関数です。
//
BOOL func_init(AviUtl::FilterPlugin* fp)
{
	// 拡張編集にアクセスするための機能を初期化します。
	exin::init();

	// フィルタプラグインを取得しておきます。
	::fp = fp;

	// フックを開始します。
	for (auto& node : collection)
	{
		auto address = exin::exedit + node.relative_address;
		::ReadProcessMemory(::GetCurrentProcess(), (void*)address,
			&node.ptr_ptr_orig_SendMessageA, sizeof(node.ptr_ptr_orig_SendMessageA), nullptr);
		::WriteProcessMemory(::GetCurrentProcess(), (void*)address,
			&node.ptr_ptr_hook_SendMessageA, sizeof(node.ptr_ptr_hook_SendMessageA), nullptr);
	}

	// detoursフックを開始します。
	{
		DetourTransactionBegin();
		DetourUpdateThread(::GetCurrentThread());

		deploy_filter_controls::init();

		DetourTransactionCommit();
	}

	return TRUE;
}

//
// 終了関数です。
//
BOOL func_exit(AviUtl::FilterPlugin* fp)
{
	// フックを終了します。
	for (auto& node : collection)
	{
		auto address = exin::exedit + node.relative_address;
		::WriteProcessMemory(::GetCurrentProcess(), (void*)address,
			&node.ptr_ptr_orig_SendMessageA, sizeof(node.ptr_ptr_orig_SendMessageA), nullptr);
	}

	// detoursフックを終了します。
	{
		DetourTransactionBegin();
		DetourUpdateThread(::GetCurrentThread());

		deploy_filter_controls::exit();

		DetourTransactionCommit();
	}

	return TRUE;
}

//
// aviutlから呼ばれるエクスポート関数です。
//
EXTERN_C AviUtl::FilterPluginDLL* WINAPI GetFilterTable()
{
	static LPCSTR check_name[] =
	{
		"コンボボックスを再構築しない",
		"コンボボックスを再描画しない",
	};

	static int check_def[] =
	{
		1,
		1,
	};

	static AviUtl::FilterPluginDLL filter =
	{
		.flag =
			AviUtl::FilterPluginDLL::Flag::AlwaysActive |
			AviUtl::FilterPluginDLL::Flag::DispFilter |
			AviUtl::FilterPluginDLL::Flag::ExInformation,
		.name = plugin_name,
		.check_n = sizeof(check_name) / sizeof(*check_name),
		.check_name = check_name,
		.check_default = check_def,
		.func_init = func_init,
		.func_exit = func_exit,
		.information = plugin_information,
	};

	return &filter;
}
