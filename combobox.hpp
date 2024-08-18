﻿#pragma once

//
// コンボボックスを操作するための機能群です。
//
namespace cb
{
	std::size_t get_count(HWND hwnd)
	{
		return (std::size_t)::SendMessageA(hwnd, CB_GETCOUNT, 0, 0);
	}

	std::size_t get_cur_sel(HWND hwnd)
	{
		return (std::size_t)::SendMessageA(hwnd, CB_GETCURSEL, 0, 0);
	}

	std::string get_text(HWND hwnd, std::size_t index)
	{
		if (index == -1) index = get_cur_sel(hwnd);

		auto text_length = ::SendMessageA(hwnd, CB_GETLBTEXTLEN, index, 0);
		if (text_length == CB_ERR) return {};
		std::string text(text_length, '\0');
		::SendMessageA(hwnd, CB_GETLBTEXT, index, (LPARAM)text.data());
		return text;
	}
}