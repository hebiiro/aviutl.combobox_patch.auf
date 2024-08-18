﻿#pragma once

//
// このクラスはアニメーション効果を管理します。
//
inline struct AnimationEffectManager : Addin
{
	LPCSTR table;
	std::unordered_map<std::string, int32_t> from_category;
	std::unordered_map<std::string, int32_t> from_name;

	//
	// コンストラクタです。
	//
	AnimationEffectManager()
	{
		addins.emplace_back(this);
	}

	//
	// カテゴリIDを返します。
	//
	int32_t get_category_id(ExEdit::Object* object, int32_t filter_index)
	{
		auto exdata = (ExEdit::Exdata::efAnimationEffect*)exin::get_exdata(object, filter_index);

		if (exdata->filter < 2)
			return exdata->filter + 1;

		return from_name[exdata->name] + 3;
	}

	//
	// 初期化処理を実行します。
	//
	virtual BOOL func_init(AviUtl::FilterPlugin* fp) override
	{
		auto names = table = (LPCSTR)(exin::exedit + 0x000C1F08);
		while (names[0])
		{
			auto length = strlen(names);
			auto temp = std::string(names, length);
			auto sep = temp.find(1);
			output_debug_string("{}, {}, {}", temp, length, sep);
			if (sep != temp.npos)
			{
				auto name = temp.substr(0, sep);
				auto category = temp.substr(sep + 1);

				auto it = from_category.find(category);
				if (it == from_category.end())
				{
					output_debug_string("カテゴリを追加します {} => {}", category, from_category.size());

					it = from_category.insert({ category, from_category.size() }).first;
				}

				from_name[name] = it->second;
			}

			names += length + 1;
		}

		return TRUE;
	}

	//
	// 後始末処理を実行します。
	//
	virtual BOOL func_exit(AviUtl::FilterPlugin* fp) override
	{
		return TRUE;
	}
} animation_effect_manager;
