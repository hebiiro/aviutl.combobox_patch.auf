#pragma once

//
// 拡張編集にアクセスするための機能群です。
//
namespace exin
{
	uint32_t exedit = 0;

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
	} variable;

	struct {
		BOOL (CDECL* update_controls)(int32_t object_index) = nullptr;
	} function;

	void init()
	{
		exedit = (decltype(exedit))::GetModuleHandleA("exedit.auf");
		variable.object_table = (decltype(variable.object_table))(exedit + 0x001E0FA4);
		variable.sorted_object_table = (decltype(variable.sorted_object_table))(exedit + 0x00168FA8);
		variable.filter_table = (decltype(variable.filter_table))(exedit + 0x00187C98);
		variable.current_object_index = (decltype(variable.current_object_index))(exedit + 0x00177A10);
		variable.exdata_table = (decltype(variable.exdata_table))(exedit + 0x001E0FA8);
		function.update_controls = (decltype(function.update_controls))(exedit + 0x000305E0);
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

	BOOL update_controls(int32_t object_index) { return function.update_controls(object_index); }
}
