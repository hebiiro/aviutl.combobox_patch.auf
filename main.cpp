#include <cassert>
#include <string>
#include <vector>
#include <unordered_map>
#include <format>
#include <filesystem>
#include "aviutl.hpp"
#include "exedit.hpp"
#include "detours.4.0.1/detours.h"
#pragma comment(lib, "detours.4.0.1/detours.lib")

#include "hive.hpp"
#include "exin.hpp"
#include "utils.hpp"
#include "addin.hpp"
#include "combobox.hpp"
#include "animation_effect_manager.hpp"
#include "create_combobox.hpp"
#include "set_current_object.hpp"

constexpr auto plugin_name = "コンボボックスパッチ";
constexpr auto plugin_information = "コンボボックスパッチ r3 by 蛇色 / originator 狐紺くろ(Tsut-ps) / X";

//
// 初期化関数です。
//
BOOL func_init(AviUtl::FilterPlugin* fp)
{
	hive.fp = fp;

	// 拡張編集にアクセスするための機能を初期化します。
	exin::init();

	for (auto& addin : addins)
		addin->func_init(fp);

	return TRUE;
}

//
// 終了関数です。
//
BOOL func_exit(AviUtl::FilterPlugin* fp)
{
	for (auto& addin : addins)
		addin->func_exit(fp);

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
