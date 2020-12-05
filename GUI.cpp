
#include "GUI.hpp"

#include "resource.h"
#include "util.hpp"
#include <windows.h>
#include <commctrl.h>
// This wndproc mess here only allows once instance of GUI per program
static GUI *gui;

LRESULT CALLBACK GUI::WindowProc(HWND wnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	LRESULT ret = CallWindowProc(gui->original_window_proc, wnd, msg, wparam, lparam);

	switch (msg)
	{
		case WM_COMMAND:
		{
			GUI::Event e = {GUI::Event::Command, wparam};
			gui->events.push_back(e);
		}
		break;
	}

	return ret;
}

static BOOL CALLBACK gui_dialog_proc(HWND dialog, UINT msg, WPARAM wparam, LPARAM lparam)
{
	switch (msg)
	{
		case WM_INITDIALOG:
			switch (gui->run_dialog_id)
			{
				case DIALOG_NEW_MAP:
					SetDlgItemText(dialog, DIALOG_NEW_MAP_WIDTH, gui->dialog_new_name);
					SetDlgItemInt(dialog, DIALOG_NEW_MAP_WIDTH, gui->dialog_new_width, FALSE);
					SetDlgItemInt(dialog, DIALOG_NEW_MAP_HEIGHT, gui->dialog_new_height, FALSE);
					break;

                case DIALOG_MAP_PROPERTIES:
                    /* Type adding */
                    SendDlgItemMessage(dialog, DIALOG_MAP_PROPERTIES_TYPE, CB_ADDSTRING, 0, (LPARAM)"0 - Normal");
                    SendDlgItemMessage(dialog, DIALOG_MAP_PROPERTIES_TYPE, CB_ADDSTRING, 0, (LPARAM)"1 - Unk");
                    SendDlgItemMessage(dialog, DIALOG_MAP_PROPERTIES_TYPE, CB_ADDSTRING, 0, (LPARAM)"2 - Unk");
                    SendDlgItemMessage(dialog, DIALOG_MAP_PROPERTIES_TYPE, CB_ADDSTRING, 0, (LPARAM)"3 - PK Zone");
                    SendDlgItemMessage(dialog, DIALOG_MAP_PROPERTIES_TYPE, CB_SETCURSEL, gui->dialog_map_type, 0);
                    /* End of type adding */
                    /* Effect adding */
                    SendDlgItemMessage(dialog, DIALOG_MAP_PROPERTIES_EFFECT, CB_ADDSTRING, 0, (LPARAM)" 0 - None");
                    SendDlgItemMessage(dialog, DIALOG_MAP_PROPERTIES_EFFECT, CB_ADDSTRING, 0, (LPARAM)" 1 - HP Drain");
                    SendDlgItemMessage(dialog, DIALOG_MAP_PROPERTIES_EFFECT, CB_ADDSTRING, 0, (LPARAM)" 2 - TP Drain");
                    SendDlgItemMessage(dialog, DIALOG_MAP_PROPERTIES_EFFECT, CB_ADDSTRING, 0, (LPARAM)" 3 - Earthquake (20-60s, 0-1 size)");
                    SendDlgItemMessage(dialog, DIALOG_MAP_PROPERTIES_EFFECT, CB_ADDSTRING, 0, (LPARAM)" 4 - Earthquake (30-60s, 0-2 size)");
                    SendDlgItemMessage(dialog, DIALOG_MAP_PROPERTIES_EFFECT, CB_ADDSTRING, 0, (LPARAM)" 5 - Earthquake ()");
                    SendDlgItemMessage(dialog, DIALOG_MAP_PROPERTIES_EFFECT, CB_ADDSTRING, 0, (LPARAM)" 6 - Earthquake (5-20s, 6-8 size)");
                    SendDlgItemMessage(dialog, DIALOG_MAP_PROPERTIES_EFFECT, CB_ADDSTRING, 0, (LPARAM)" 7 - ?");
                    SendDlgItemMessage(dialog, DIALOG_MAP_PROPERTIES_EFFECT, CB_ADDSTRING, 0, (LPARAM)" 8 - ?");
                    SendDlgItemMessage(dialog, DIALOG_MAP_PROPERTIES_EFFECT, CB_ADDSTRING, 0, (LPARAM)" 9 - ?");
                    SendDlgItemMessage(dialog, DIALOG_MAP_PROPERTIES_EFFECT, CB_ADDSTRING, 0, (LPARAM)"10 - ?");
                    SendDlgItemMessage(dialog, DIALOG_MAP_PROPERTIES_EFFECT, CB_SETCURSEL, gui->dialog_map_effect, 0);
                    /* End of effect adding */

					SetDlgItemText(dialog, DIALOG_MAP_PROPERTIES_NAME, gui->dialog_map_name);
                    SetDlgItemInt(dialog, DIALOG_MAP_PROPERTIES_WIDTH, gui->dialog_map_width, FALSE);
                    SetDlgItemInt(dialog, DIALOG_MAP_PROPERTIES_HEIGHT, gui->dialog_map_height, FALSE);
                    SetDlgItemInt(dialog, DIALOG_MAP_PROPERTIES_RESET_X, gui->dialog_map_reset_x, FALSE);
                    SetDlgItemInt(dialog, DIALOG_MAP_PROPERTIES_RESET_Y, gui->dialog_map_reset_y, FALSE);
                    if (gui->dialog_map_map_available)
						SendMessage(GetDlgItem(dialog, DIALOG_MAP_PROPERTIES_MINIMAP), BM_SETCHECK, BST_CHECKED, 0);
                    if (gui->dialog_map_can_scroll)
						SendMessage(GetDlgItem(dialog, DIALOG_MAP_PROPERTIES_TELEPORT), BM_SETCHECK, BST_CHECKED, 0);

                    /* Music setting */
                    SetDlgItemInt(dialog, DIALOG_MAP_PROPERTIES_MUSIC_TRACK, gui->dialog_map_music_track, FALSE);
                    SetDlgItemInt(dialog, DIALOG_MAP_PROPERTIES_MUSIC_AMBIENT, gui->dialog_map_music_ambient, FALSE);
                    /* Music control adding */
                    SendDlgItemMessage(dialog, DIALOG_MAP_PROPERTIES_MUSIC_CONTROL, CB_ADDSTRING, 0, (LPARAM)"0 - ?");
                    SendDlgItemMessage(dialog, DIALOG_MAP_PROPERTIES_MUSIC_CONTROL, CB_ADDSTRING, 0, (LPARAM)"1 - ?");
                    SendDlgItemMessage(dialog, DIALOG_MAP_PROPERTIES_MUSIC_CONTROL, CB_ADDSTRING, 0, (LPARAM)"2 - ?");
                    SendDlgItemMessage(dialog, DIALOG_MAP_PROPERTIES_MUSIC_CONTROL, CB_ADDSTRING, 0, (LPARAM)"3 - ?");
                    SendDlgItemMessage(dialog, DIALOG_MAP_PROPERTIES_MUSIC_CONTROL, CB_ADDSTRING, 0, (LPARAM)"4 - ?");
                    SendDlgItemMessage(dialog, DIALOG_MAP_PROPERTIES_MUSIC_CONTROL, CB_ADDSTRING, 0, (LPARAM)"5 - ?");
                    SendDlgItemMessage(dialog, DIALOG_MAP_PROPERTIES_MUSIC_CONTROL, CB_ADDSTRING, 0, (LPARAM)"6 - ?");
                    /* End of music control adding */
                    SendDlgItemMessage(dialog, DIALOG_MAP_PROPERTIES_MUSIC_CONTROL, CB_SETCURSEL, gui->dialog_map_music_control, 0);
                    /* End of music setting */
                    break;

				case DIALOG_EDIT_WARP:
					SetDlgItemInt(dialog, DIALOG_EDIT_WARP_MAP, gui->dialog_warp_map, FALSE);
					SetDlgItemInt(dialog, DIALOG_EDIT_WARP_X, gui->dialog_warp_x, FALSE);
					SetDlgItemInt(dialog, DIALOG_EDIT_WARP_Y, gui->dialog_warp_y, FALSE);
					SetDlgItemInt(dialog, DIALOG_EDIT_WARP_LEVEL, gui->dialog_warp_level, FALSE);

					if (gui->dialog_warp_door)
						SendMessage(GetDlgItem(dialog, DIALOG_EDIT_WARP_DOOR), BM_SETCHECK, BST_CHECKED, 0);

					SetDlgItemInt(dialog, DIALOG_EDIT_WARP_KEY, gui->dialog_warp_key, FALSE);

					break;

                case DIALOG_ITEM_SPAWN_LIST:
                    for (std::vector<EO_Map::Chest>::iterator i = gui->chest_spawns.begin(); i != gui->chest_spawns.end(); ++i)
                    {
                        std::string buffer;
                        buffer = "(";
                        buffer += util::to_string(i->slot);
                        buffer += ") ";
                        buffer += util::to_string(i->item);
                        buffer += " : ";
                        buffer += util::to_string(static_cast<int>(i->amount));
                        buffer += " Time: ";
                        buffer += util::to_string(i->time);
                        SendDlgItemMessage(dialog, DIALOG_ITEM_SPAWN_LIST_ITEMS, CB_ADDSTRING, 0, (LPARAM)buffer.c_str());
                    }
                    break;

                case DIALOG_NPC_SPAWN_LIST:
                    for (std::vector<EO_Map::NPC>::iterator i = gui->npc_spawns.begin(); i != gui->npc_spawns.end(); ++i)
                    {
                        std::string buffer;
                        buffer = util::to_string(i->id);
                        buffer += " : ";
                        buffer += util::to_string(static_cast<int>(i->amount));
                        SendDlgItemMessage(dialog, DIALOG_NPC_SPAWN_LIST_NPCS, CB_ADDSTRING, 0, (LPARAM)buffer.c_str());
                    }
                    break;

                case DIALOG_EDIT_ITEM_SPAWN:
                    SetDlgItemInt(dialog, DIALOG_EDIT_ITEM_SPAWN_ID, gui->dialog_current_item_spawn.item, FALSE);
                    SetDlgItemInt(dialog, DIALOG_EDIT_ITEM_SPAWN_AMOUNT, gui->dialog_current_item_spawn.amount, FALSE);
                    SetDlgItemInt(dialog, DIALOG_EDIT_ITEM_SPAWN_KEY, gui->dialog_current_item_spawn.key, FALSE);
                    SetDlgItemInt(dialog, DIALOG_EDIT_ITEM_SPAWN_TIME, gui->dialog_current_item_spawn.time, FALSE);
                    SetDlgItemInt(dialog, DIALOG_EDIT_ITEM_SPAWN_SLOT, gui->dialog_current_item_spawn.slot, FALSE);
                    break;

                case DIALOG_EDIT_NPC_SPAWN:
                    SetDlgItemInt(dialog, DIALOG_EDIT_NPC_SPAWN_ID, gui->dialog_current_npc_spawn.id, FALSE);
                    SetDlgItemInt(dialog, DIALOG_EDIT_NPC_SPAWN_SPEED, gui->dialog_current_npc_spawn.spawn_type, FALSE);
                    SetDlgItemInt(dialog, DIALOG_EDIT_NPC_SPAWN_RESPAWN, gui->dialog_current_npc_spawn.spawn_time, FALSE);
                    SetDlgItemInt(dialog, DIALOG_EDIT_NPC_SPAWN_AMOUNT, gui->dialog_current_npc_spawn.amount, FALSE);
                    break;

				case DIALOG_EDIT_SIGN:
					SetDlgItemText(dialog, DIALOG_EDIT_SIGN_TITLE, gui->dialog_sign_title.c_str());
					SetDlgItemText(dialog, DIALOG_EDIT_SIGN_MESSAGE, gui->dialog_sign_message.c_str());
					break;
			}
			break;


		case WM_COMMAND:
			switch (wparam)
			{
				case DIALOG_NEW_MAP_OK:
					GetDlgItemText(dialog, DIALOG_NEW_MAP_NAME, gui->dialog_new_name, sizeof(gui->dialog_new_name));
					gui->dialog_new_name[sizeof(gui->dialog_new_name)-1] = '\0';
					gui->dialog_new_width = GetDlgItemInt(dialog, DIALOG_NEW_MAP_WIDTH, 0, FALSE);
					gui->dialog_new_height = GetDlgItemInt(dialog, DIALOG_NEW_MAP_HEIGHT, 0, FALSE);
					EndDialog(dialog, 1);
					break;

				case DIALOG_NEW_MAP_CANCEL:
					EndDialog(dialog, 0);
					break;

                case DIALOG_MAP_PROPERTIES_MINIMAP:
                    gui->dialog_map_map_available = !gui->dialog_map_map_available;
                    break;

                case DIALOG_MAP_PROPERTIES_TELEPORT:
                    gui->dialog_map_can_scroll = !gui->dialog_map_can_scroll;
                    break;

                case DIALOG_MAP_PROPERTIES_OK:
                    //GetDlgItemInt(dialog, DIALOG_NEW_MAP_WIDTH, 0, FALSE);
                    gui->dialog_map_type          = SendDlgItemMessage(dialog, DIALOG_MAP_PROPERTIES_TYPE, CB_GETCURSEL, 0, 0);
                    gui->dialog_map_effect        = SendDlgItemMessage(dialog, DIALOG_MAP_PROPERTIES_EFFECT, CB_GETCURSEL, 0, 0);

					GetDlgItemText(dialog, DIALOG_MAP_PROPERTIES_NAME, gui->dialog_map_name, sizeof(gui->dialog_map_name));
					gui->dialog_map_name[sizeof(gui->dialog_map_name)-1] = '\0';

                    gui->dialog_map_width = GetDlgItemInt(dialog, DIALOG_MAP_PROPERTIES_WIDTH, 0, FALSE);
                    gui->dialog_map_height = GetDlgItemInt(dialog, DIALOG_MAP_PROPERTIES_HEIGHT, 0, FALSE);
                    gui->dialog_map_reset_x = GetDlgItemInt(dialog, DIALOG_MAP_PROPERTIES_RESET_X, 0, FALSE);
                    gui->dialog_map_reset_y = GetDlgItemInt(dialog, DIALOG_MAP_PROPERTIES_RESET_Y, 0, FALSE);

                    gui->dialog_map_music_control = SendDlgItemMessage(dialog, DIALOG_MAP_PROPERTIES_MUSIC_CONTROL, CB_GETCURSEL, 0, 0);

                    gui->dialog_map_music_track   = GetDlgItemInt(dialog, DIALOG_MAP_PROPERTIES_MUSIC_TRACK, 0, FALSE);
                    gui->dialog_map_music_ambient = GetDlgItemInt(dialog, DIALOG_MAP_PROPERTIES_MUSIC_AMBIENT, 0, FALSE);

                    EndDialog(dialog, 1);
                    break;

                case DIALOG_MAP_PROPERTIES_CANCEL:
                    EndDialog(dialog, 0);
                    break;

				case DIALOG_EDIT_WARP_DOOR:
					gui->dialog_warp_door = !gui->dialog_warp_door;
					break;

				case DIALOG_EDIT_WARP_OK:
					gui->dialog_warp_map = GetDlgItemInt(dialog, DIALOG_EDIT_WARP_MAP, 0, FALSE);
					gui->dialog_warp_x = GetDlgItemInt(dialog, DIALOG_EDIT_WARP_X, 0, FALSE);
					gui->dialog_warp_y = GetDlgItemInt(dialog, DIALOG_EDIT_WARP_Y, 0, FALSE);
					gui->dialog_warp_level = GetDlgItemInt(dialog, DIALOG_EDIT_WARP_LEVEL, 0, FALSE);
					gui->dialog_warp_key = GetDlgItemInt(dialog, DIALOG_EDIT_WARP_KEY, 0, FALSE);
					EndDialog(dialog, 1);
					break;

				case DIALOG_EDIT_WARP_CANCEL:
					EndDialog(dialog, 0);
					break;


				case DIALOG_ABOUT_CLOSE:
					EndDialog(dialog, 0);
					break;


                case DIALOG_ITEM_SPAWN_LIST_ADD:
                {
                    gui->dialog_current_item_spawn = EO_Map::Chest();
                    gui->dialog_item_spawn_action = 1;

                    EndDialog(dialog, 1);
                }
                break;

                case DIALOG_ITEM_SPAWN_LIST_DELETE:
                {
                    gui->dialog_selected_item_spawn_id = SendDlgItemMessage(dialog, DIALOG_ITEM_SPAWN_LIST_ITEMS, CB_GETCURSEL, 0, 0);
                    if (gui->dialog_selected_item_spawn_id == CB_ERR) break;
                    gui->dialog_current_item_spawn = gui->chest_spawns[gui->dialog_selected_item_spawn_id];
                    gui->dialog_item_spawn_action = 2;

                    EndDialog(dialog, 1);
                }
                break;

                case DIALOG_ITEM_SPAWN_LIST_EDIT:
                {
                    gui->dialog_selected_item_spawn_id = SendDlgItemMessage(dialog, DIALOG_ITEM_SPAWN_LIST_ITEMS, CB_GETCURSEL, 0, 0);
                    if (gui->dialog_selected_item_spawn_id == CB_ERR) break;
                    gui->dialog_current_item_spawn = gui->chest_spawns[gui->dialog_selected_item_spawn_id];
                    gui->dialog_item_spawn_action = 3;

                    EndDialog(dialog, 1);
                }
                break;

                case DIALOG_ITEM_SPAWN_LIST_SAVE:
                    EndDialog(dialog, 0);
					break;

                case DIALOG_EDIT_ITEM_SPAWN_OK:
                    gui->dialog_edited_item_spawn = gui->dialog_current_item_spawn;
                    gui->dialog_edited_item_spawn.item = GetDlgItemInt(dialog, DIALOG_EDIT_ITEM_SPAWN_ID, 0, FALSE);
                    gui->dialog_edited_item_spawn.amount = GetDlgItemInt(dialog, DIALOG_EDIT_ITEM_SPAWN_AMOUNT, 0, FALSE);
                    gui->dialog_edited_item_spawn.key = GetDlgItemInt(dialog, DIALOG_EDIT_ITEM_SPAWN_KEY, 0, FALSE);
                    gui->dialog_edited_item_spawn.time = GetDlgItemInt(dialog, DIALOG_EDIT_ITEM_SPAWN_TIME, 0, FALSE);
                    gui->dialog_edited_item_spawn.slot = GetDlgItemInt(dialog, DIALOG_EDIT_ITEM_SPAWN_SLOT, 0, FALSE);
                    EndDialog(dialog, 1);
                    break;

                case DIALOG_EDIT_ITEM_SPAWN_CANCEL:
                    EndDialog(dialog, 0);
                    break;

                case DIALOG_NPC_SPAWN_LIST_ADD:
                {
                    gui->dialog_current_npc_spawn = EO_Map::NPC();
                    gui->dialog_npc_spawn_action = 1;

                    EndDialog(dialog, 1);
                }
                break;

                case DIALOG_NPC_SPAWN_LIST_DELETE:
                {
                    gui->dialog_selected_npc_spawn_id = SendDlgItemMessage(dialog, DIALOG_NPC_SPAWN_LIST_NPCS, CB_GETCURSEL, 0, 0);
                    if (gui->dialog_selected_npc_spawn_id == CB_ERR) break;
                    gui->dialog_current_npc_spawn = gui->npc_spawns[gui->dialog_selected_npc_spawn_id];
                    gui->dialog_npc_spawn_action = 2;

                    EndDialog(dialog, 1);
                }
                break;

                case DIALOG_NPC_SPAWN_LIST_EDIT:
                {
                    gui->dialog_selected_npc_spawn_id = SendDlgItemMessage(dialog, DIALOG_NPC_SPAWN_LIST_NPCS, CB_GETCURSEL, 0, 0);
                    if (gui->dialog_selected_npc_spawn_id == CB_ERR) break;
                    gui->dialog_current_npc_spawn = gui->npc_spawns[gui->dialog_selected_npc_spawn_id];
                    gui->dialog_npc_spawn_action = 3;

                    EndDialog(dialog, 1);
                }
                break;

                case DIALOG_NPC_SPAWN_LIST_SAVE:
                    EndDialog(dialog, 0);
					break;

                case DIALOG_EDIT_NPC_SPAWN_OK:
                    gui->dialog_edited_npc_spawn = gui->dialog_current_npc_spawn;
                    gui->dialog_edited_npc_spawn.id = GetDlgItemInt(dialog, DIALOG_EDIT_NPC_SPAWN_ID, 0, FALSE);
                    gui->dialog_edited_npc_spawn.spawn_type = GetDlgItemInt(dialog, DIALOG_EDIT_NPC_SPAWN_SPEED, 0, FALSE);
                    gui->dialog_edited_npc_spawn.spawn_time = GetDlgItemInt(dialog, DIALOG_EDIT_NPC_SPAWN_RESPAWN, 0, FALSE);
                    gui->dialog_edited_npc_spawn.amount = GetDlgItemInt(dialog, DIALOG_EDIT_NPC_SPAWN_AMOUNT, 0, FALSE);
                    EndDialog(dialog, 1);
                    break;

                case DIALOG_EDIT_NPC_SPAWN_CANCEL:
                    EndDialog(dialog, 0);
                    break;

                case DIALOG_EDIT_SIGN_OK:
                {
                    char *ttl = new char[100];
                    char *msg = new char[100];
					/*gui->dialog_sign_title = */GetDlgItemText(dialog, DIALOG_EDIT_SIGN_TITLE, ttl, 100);
					/*gui->dialog_sign_message = */GetDlgItemText(dialog, DIALOG_EDIT_SIGN_MESSAGE, msg, 100);
					gui->dialog_sign_title = ttl;
					gui->dialog_sign_message = msg;
					EndDialog(dialog, 1);
                }
                break;

				case DIALOG_EDIT_SIGN_CANCEL:
					EndDialog(dialog, 0);
					break;
			}
			break;


		case WM_CLOSE:
			EndDialog(dialog, wparam);
			break;

		default:
			DefWindowProc(dialog, msg, wparam, lparam);
	}

	return FALSE;
}

GUI::GUI(a5::Display &display, a5::Display &pal_display)
{
	gui = this;

	this->display = &display;
	this->pal_display = &pal_display;

	this->wnd = al_get_win_window_handle(display);
	this->pal_wnd = al_get_win_window_handle(pal_display);

	DWORD map_display_x = 0, map_display_y = 0;
	DWORD map_display_w = 0, map_display_h = 0;
	DWORD map_display_x_type = REG_DWORD, map_display_y_type = REG_DWORD;
	DWORD map_display_w_type = REG_DWORD, map_display_h_type = REG_DWORD;
	DWORD map_display_x_size = sizeof(DWORD), map_display_y_size = sizeof(DWORD);
	DWORD map_display_w_size = sizeof(DWORD), map_display_h_size = sizeof(DWORD);

	int map_display_x_int = 0, map_display_y_int = 0;

	int ret;

	if ((ret = RegCreateKeyEx(HKEY_CURRENT_USER, "Software\\EOMap2", 0, 0, 0, KEY_READ | KEY_WRITE, 0, &registry, 0)) != ERROR_SUCCESS)
	{
		registry = 0;
	}

	DWORD x1 = 0, y1 = 0, x2 = 0, y2 = 0;
	ALLEGRO_MONITOR_INFO info;

	for (int i = 0; i < al_get_num_video_adapters(); ++i)
	{
		al_get_monitor_info(i, &info);
		x1 = std::min(x1, DWORD(info.x1));
		y1 = std::min(x1, DWORD(info.y1));
		x2 = std::max(x1, DWORD(info.x2));
		y2 = std::max(x1, DWORD(info.y2));
	}

	if (registry)
	{
		RegQueryValueEx(registry, "MapWinPosX", 0, &map_display_x_type, reinterpret_cast<BYTE *>(&map_display_x), &map_display_x_size);
		if (map_display_x_type != REG_DWORD || map_display_x < x1) goto default_position;
		RegQueryValueEx(registry, "MapWinPosY", 0, &map_display_y_type, reinterpret_cast<BYTE *>(&map_display_y), &map_display_y_size);
		if (map_display_y_type != REG_DWORD || map_display_y < y1) goto default_position;
		RegQueryValueEx(registry, "MapWinW", 0, &map_display_w_type, reinterpret_cast<BYTE *>(&map_display_w), &map_display_w_size);
		if (map_display_w_type != REG_DWORD || (map_display_x) >= x2) goto default_position;
		RegQueryValueEx(registry, "MapWinH", 0, &map_display_h_type, reinterpret_cast<BYTE *>(&map_display_h), &map_display_h_size);
		if (map_display_h_type != REG_DWORD || (map_display_y) >= y2) goto default_position;

		SetWindowPos(this->wnd, HWND_TOP, map_display_x, map_display_y, map_display_w, map_display_h, 0);

		RegQueryValueEx(registry, "PalWinPosX", 0, &map_display_x_type, reinterpret_cast<BYTE *>(&map_display_x), &map_display_x_size);
		if (map_display_x_type != REG_DWORD || map_display_x < x1) goto default_position;
		RegQueryValueEx(registry, "PalWinPosY", 0, &map_display_y_type, reinterpret_cast<BYTE *>(&map_display_y), &map_display_y_size);
		if (map_display_y_type != REG_DWORD || map_display_y < y1) goto default_position;
		RegQueryValueEx(registry, "PalWinW", 0, &map_display_w_type, reinterpret_cast<BYTE *>(&map_display_w), &map_display_w_size);
		if (map_display_w_type != REG_DWORD || (map_display_x) >= x2) goto default_position;
		RegQueryValueEx(registry, "PalWinH", 0, &map_display_h_type, reinterpret_cast<BYTE *>(&map_display_h), &map_display_h_size);
		if (map_display_h_type != REG_DWORD || (map_display_y) >= y2) goto default_position;

		SetWindowPos(this->pal_wnd, HWND_TOP, map_display_x, map_display_y, map_display_w, map_display_h, 0);
	}
	else
	{
		default_position:
		al_get_window_position(*this->display, &map_display_x_int, &map_display_y_int);
		al_set_window_position(*this->display, map_display_x_int, map_display_y_int/2 - 32);
		al_set_window_position(*this->pal_display, map_display_x_int, map_display_y_int/2 + this->display->Height() + 32);
	}

	SetWindowLongPtr(this->pal_wnd, GWL_STYLE, WS_THICKFRAME | WS_OVERLAPPED | WS_CAPTION);
	ShowWindow(this->wnd, SW_SHOW);
	ShowWindow(this->pal_wnd, SW_SHOW);
	SetForegroundWindow(this->wnd);
	this->menu = LoadMenu(GetModuleHandle(0), MAKEINTRESOURCE(MENU_BASE));
	this->original_window_proc = reinterpret_cast<WNDPROC>(SetWindowLongPtr(this->wnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(GUI::WindowProc)));

	// Menu causes issues with dxvk
	SetMenu(this->wnd, this->menu);
}

int GUI::RunDialog(int dialogid)
{
	run_dialog_id = dialogid;

	return DialogBox(GetModuleHandle(0), MAKEINTRESOURCE(dialogid), this->wnd, gui_dialog_proc);
}

void GUI::MenuOff()
{
	SetMenu(this->wnd, nullptr);
}

void GUI::MenuOn()
{
	SetMenu(this->wnd, this->menu);
}

static char gui_open_filename[4096];

const char *GUI::OpenFile()
{
	OPENFILENAME ofn;

	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = this->wnd;
	ofn.lpstrFile = gui_open_filename;
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = sizeof(gui_open_filename);
	ofn.lpstrTitle = "Open Map File";
	ofn.lpstrFilter = "Endless Map Files (*.emf)\0*.EMF\0All Files (*.*)\0*.*\0";
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

	if (GetOpenFileName(&ofn) == TRUE)
	{
		return ofn.lpstrFile;
	}
	else
	{
		return 0;
	}
}

const char *GUI::SaveFile()
{
	OPENFILENAME ofn;

	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = this->wnd;
	ofn.lpstrFile = gui_open_filename;
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = sizeof(gui_open_filename);
	ofn.lpstrTitle = "Save Map File";
	ofn.lpstrFilter = "Endless Map Files (*.emf)\0*.EMF\0All Files (*.*)\0*.*\0";
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;

	if (GetSaveFileName(&ofn) == TRUE)
	{
		return ofn.lpstrFile;
	}
	else
	{
		return 0;
	}
}

GUI::~GUI()
{
	if (this->registry)
	{
		RECT rect;

		GetWindowRect(this->wnd, &rect);
		int map_display_x = rect.left;
		int map_display_y = rect.top;
		RegSetValueEx(this->registry, "MapWinPosX", 0, REG_DWORD, reinterpret_cast<BYTE *>(&map_display_x), sizeof(DWORD));
		RegSetValueEx(this->registry, "MapWinPosY", 0, REG_DWORD, reinterpret_cast<BYTE *>(&map_display_y), sizeof(DWORD));

		map_display_x = rect.right - rect.left;
		map_display_y = rect.bottom - rect.top;
		RegSetValueEx(this->registry, "MapWinW", 0, REG_DWORD, reinterpret_cast<BYTE *>(&map_display_x), sizeof(DWORD));
		RegSetValueEx(this->registry, "MapWinH", 0, REG_DWORD, reinterpret_cast<BYTE *>(&map_display_y), sizeof(DWORD));

		GetWindowRect(this->pal_wnd, &rect);
		map_display_x = rect.left;
		map_display_y = rect.top;
		RegSetValueEx(this->registry, "PalWinPosX", 0, REG_DWORD, reinterpret_cast<BYTE *>(&map_display_x), sizeof(DWORD));
		RegSetValueEx(this->registry, "PalWinPosY", 0, REG_DWORD, reinterpret_cast<BYTE *>(&map_display_y), sizeof(DWORD));

		map_display_x = rect.right - rect.left;
		map_display_y = rect.bottom - rect.top;
		RegSetValueEx(this->registry, "PalWinW", 0, REG_DWORD, reinterpret_cast<BYTE *>(&map_display_x), sizeof(DWORD));
		RegSetValueEx(this->registry, "PalWinH", 0, REG_DWORD, reinterpret_cast<BYTE *>(&map_display_y), sizeof(DWORD));
	}

	/*
	if (this->menu)
	{
		SetMenu(this->wnd, 0);
		DestroyMenu(this->menu);
		this->menu = 0;
	}
	*/

	SetWindowLongPtr(this->wnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(this->original_window_proc));
}

void GUI::SetMenuEnabled(int resid, bool enabled)
{
	MENUITEMINFO mii;
	ZeroMemory(&mii, sizeof(mii));
	mii.cbSize = sizeof(mii);
	mii.fMask = MIIM_STATE;
	GetMenuItemInfo(this->menu, resid, false, &mii);

	if (enabled)
	{
		mii.fState &= ~MFS_GRAYED;
		mii.fState |= MFS_ENABLED;
	}
	else
	{
		mii.fState &= ~MFS_ENABLED;
		mii.fState |= MFS_GRAYED;
	}

	SetMenuItemInfo(this->menu, resid, false, &mii);
/*
	int idx = -1;
	al_find_menu_item(al_menu, resid, nullptr, &idx);
	int flags = al_get_menu_item_flags(al_menu, idx);

	if (enabled)
		flags = flags & ~ALLEGRO_MENU_ITEM_DISABLED;
	else
		flags = flags | ALLEGRO_MENU_ITEM_DISABLED;

	al_set_menu_item_flags(al_menu, idx, flags);
*/
}

void GUI::SetMenuChecked(int resid, bool enabled)
{
	MENUITEMINFO mii;
	ZeroMemory(&mii, sizeof(mii));
	mii.cbSize = sizeof(mii);
	mii.fMask = MIIM_STATE;
	GetMenuItemInfo(this->menu, resid, false, &mii);

	if (enabled)
	{
		mii.fState &= ~MFS_UNCHECKED;
		mii.fState |= MFS_CHECKED;
	}
	else
	{
		mii.fState &= ~MFS_CHECKED;
		mii.fState |= MFS_UNCHECKED;
	}

	SetMenuItemInfo(this->menu, resid, false, &mii);
}

void GUI::Message(const char *title, const char *message)
{
	MessageBox(0, message, title, MB_OK | MB_TASKMODAL);
}
