#pragma once

inline struct Hive
{
	struct HookFlagManager {
		HookFlagManager() { hive.hook_flag++; }
		~HookFlagManager() { hive.hook_flag--; }
	};

	AviUtl::FilterPlugin* fp = nullptr;
	int32_t hook_flag = 0;

} hive;
