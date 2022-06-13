#pragma once

#include <cinttypes>
#include <string>

struct sl_title
{
	uint64_t app_id;
	const wchar_t* dirname;
	const wchar_t* exe_relpath;
};

static const uint64_t APP_ID_DS1R = 570940;
static const uint64_t APP_ID_DS2SOTFS = 335300;
static const uint64_t APP_ID_DS3 = 374320;
static const uint64_t APP_ID_SEKIRO = 814380;
static const uint64_t APP_ID_ELDENRING = 1245620;

static const sl_title SL_TITLE_DS1R{ 570940, L"DARK SOULS REMASTERED", L"DarkSoulsRemastered.exe" };
static const sl_title SL_TITLE_DS2SOTFS{ 335300, L"Dark Souls II Scholar of the First Sin", L"Game\\DarkSoulsII.exe" };
static const sl_title SL_TITLE_DS3{ 374320, L"DARK SOULS III", L"Game\\DarkSoulsIII.exe" };
static const sl_title SL_TITLE_SEKIRO{ 814380, L"Sekiro", L"sekiro.exe" };
static const sl_title SL_TITLE_ELDENRING{ 1245620, L"ELDEN RING", L"Game\\eldenring.exe" };
