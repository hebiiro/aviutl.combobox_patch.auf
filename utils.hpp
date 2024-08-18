#pragma once

#ifdef _DEBUG
#define output_debug_string(...) ::OutputDebugStringA(std::format("{}({}) : {}\n", __FILE__, __LINE__, std::format(__VA_ARGS__)).c_str())
#else
#define output_debug_string(...) ((void)0)
#endif

struct FilterControl
{
	inline static constexpr int32_t c_max_size = 48;

	uint32_t u1;
	uint32_t u2;
	HWND button;
	HWND stat;
	HWND checkbox;
	HWND combobox;
};

FilterControl* get_filter_controls()
{
	return (FilterControl*)(exin::exedit + 0x00168A18);
}

int32_t find_combobox_index(HWND hwnd)
{
	auto filter_controls = get_filter_controls();

	for (int32_t i = 0; i < FilterControl::c_max_size; i++)
		if (filter_controls[i].combobox == hwnd) return i;

	return -1;
}

HWND get_combobox(int32_t index)
{
	auto filter_controls = get_filter_controls();

	return filter_controls[index].combobox;
}

void swap_combobox(int32_t a, int32_t b)
{
	auto filter_controls = get_filter_controls();

	auto a_id = ::GetWindowLongPtrA(filter_controls[a].combobox, GWLP_ID);
	auto b_id = ::GetWindowLongPtrA(filter_controls[b].combobox, GWLP_ID);
	::SetWindowLongPtrA(filter_controls[a].combobox, GWLP_ID, b_id);
	::SetWindowLongPtrA(filter_controls[b].combobox, GWLP_ID, a_id);

	std::swap(filter_controls[a].combobox, filter_controls[b].combobox);
}

int32_t get_category_id(HWND hwnd)
{
	return (int32_t)::GetPropA(hwnd, "combobox_patch.category_id");
}

int32_t set_category_id(HWND hwnd, int32_t category_id)
{
	return (int32_t)::SetPropA(hwnd, "combobox_patch.category_id", (HANDLE)category_id);
}

int32_t find_cached_combobox_index(int32_t combobox_index,  int32_t filter_category_id)
{
	auto filter_controls = get_filter_controls();

	// combobox_index以降のコンボボックスを走査します。
	for (int32_t i = combobox_index + 1; i < FilterControl::c_max_size; i++)
	{
		// コンボボックスのカテゴリを取得します。
		auto category_id = get_category_id(filter_controls[i].combobox);

		// カテゴリが一致する場合は
		if (category_id == filter_category_id)
			return i; // そのインデックスを返します。
	}

	return -1;

}

int32_t find_nocached_combobox_index(int32_t combobox_index)
{
	auto filter_controls = get_filter_controls();

	// combobox_index以降のコンボボックスを走査します。
	for (int32_t i = combobox_index + 1; i < FilterControl::c_max_size; i++)
	{
		// コンボボックスのカテゴリを取得します。
		auto category_id = get_category_id(filter_controls[i].combobox);

		// カテゴリが設定されていない場合は
		if (category_id == 0)
			return i; // そのインデックスを返します。
	}

	return -1;

}

#ifdef _DEBUG
void dump_filter_controls()
{
#if 0
	for (int32_t i = 0; i < FilterControl::c_max_size; i++)
	{
		// コンボボックスを取得します。
		auto hwnd = get_combobox(i);

		// コンボボックスのカテゴリを取得します。
		auto cached_category_id = get_category_id(hwnd);

		output_debug_string("{}, {:#010x}, {}", i, (uint32_t)hwnd, cached_category_id);
	}
#endif
}
#else
void dump_filter_controls() {}
#endif

struct FilterPtr
{
	int32_t filter_index;
	int32_t filter_id;
	ExEdit::Filter* filter;

	auto index() const { return filter_index; }
	auto id() const { return filter_id; }
	auto get() const { return filter; }

	//
	// このフィルタがアニメーション効果の場合はTRUEを返します。
	//
	bool is_animation_effect() const
	{
		return strcmp(filter->name, "アニメーション効果") == 0;
	}
};

struct ObjectPtr
{
	int32_t object_index;
	ExEdit::Object* object;

	//
	// オブジェクトポインタを初期化します。
	//
	BOOL init(int32_t _object_index)
	{
		object_index = _object_index;
		if (object_index < 0) return FALSE;

		object = exin::get_object(object_index);
		if (!object) return FALSE;

		if (object->index_midpt_leader >= 0)
		{
			object_index = object->index_midpt_leader;
			if (object_index < 0) return FALSE;

			object = exin::get_object(object_index);
			if (!object) return FALSE;
		}

		return TRUE;
	}

	auto index() const { return object_index; }
	auto get() const { return object; }
	auto get_filter_count() const { return object->countFilters(); }

	//
	// 指定されたインデックスにあるフィルタを返します。
	//
	FilterPtr get_filter(int32_t filter_index) const
	{
		auto filter_id = object->filter_param[filter_index].id;
		auto filter = exin::get_filter(filter_id);
		return { filter_index, filter_id, filter };
	}

	//
	// 指定されたコンボボックスを持つアニメーション効果を返します。
	//
	FilterPtr get_animation_effect(HWND hwnd) const
	{
		// コンボボックスのインデックスです。
		int32_t combobox_index = 0;

		// このオブジェクトのフィルタを走査します。
		int32_t c = get_filter_count();
		for (int32_t i = 0; i < c; i++)
		{
			// このオブジェクトのフィルタを取得します。
			auto filter = get_filter(i);

			// フィルタがアニメーション効果の場合は
			if (filter.is_animation_effect())
			{
				// コンボボックスが一致する場合はこのフィルタを返します。
				if (get_combobox(combobox_index) == hwnd) return filter;
			}

			// コンボボックスのインデックスを進めます。
			combobox_index += filter.get()->check_n;
		}

		return {};
	}
};
