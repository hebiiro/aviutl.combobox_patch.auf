#pragma once

struct Addin
{
	virtual BOOL func_init(AviUtl::FilterPlugin* fp) = 0;
	virtual BOOL func_exit(AviUtl::FilterPlugin* fp) = 0;
};

std::vector<Addin*> addins;
