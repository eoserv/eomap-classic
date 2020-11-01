#ifndef GUI_HPP_INCLUDED
#define GUI_HPP_INCLUDED

#include "EO_Map.hpp"
#include "common.hpp"

class GUI
{
	protected:
		a5::Display *display, *pal_display;

		HKEY registry;
		HWND wnd, pal_wnd;
		HMENU menu;
		WNDPROC original_window_proc;

		static LRESULT CALLBACK WindowProc(HWND wnd, UINT msg, WPARAM wparam, LPARAM lparam);

	public:
		struct Event
		{
			enum Sub
			{
				Command
			};

			Sub subtype;
			WPARAM resid;
		};

		std::list<Event> events;

		int run_dialog_id;

		char dialog_new_name[25];

		unsigned char dialog_new_width;
		unsigned char dialog_new_height;

        unsigned short dialog_map_type;
        unsigned short dialog_map_effect;
		unsigned char  dialog_map_width;
		unsigned char  dialog_map_height;
		unsigned char  dialog_map_reset_x;
        unsigned char  dialog_map_reset_y;
        bool           dialog_map_map_available;
        bool           dialog_map_can_scroll;
        unsigned char  dialog_map_music_track;
        unsigned char  dialog_map_music_ambient;
        unsigned char  dialog_map_music_control;
		char           dialog_map_name[25];

		unsigned short dialog_warp_map;
		unsigned char dialog_warp_x;
		unsigned char dialog_warp_y;
		unsigned char dialog_warp_level;
		bool dialog_warp_door;
		int dialog_warp_key;

		int dialog_item_spawn_action;
		int dialog_selected_item_spawn_id;
		EO_Map::Chest dialog_current_item_spawn;
		EO_Map::Chest dialog_edited_item_spawn;
		std::vector<EO_Map::Chest> chest_spawns;

		int dialog_npc_spawn_action;
		int dialog_selected_npc_spawn_id;
		EO_Map::NPC dialog_current_npc_spawn;
		EO_Map::NPC dialog_edited_npc_spawn;
		std::vector<EO_Map::NPC> npc_spawns;

        std::string dialog_test;

        std::string dialog_sign_title;
        std::string dialog_sign_message;

		GUI(a5::Display &display, a5::Display &pal_display);

		int RunDialog(int dialogid);
		void SetMenuEnabled(int resid, bool enabled);
		void SetMenuChecked(int resid, bool enabled);

		void Message(const char *title, const char *message);

		void SwitchIn();
		void SwitchOut();

		void MenuOff();
		void MenuOn();

		const char *OpenFile();
		const char *SaveFile();

		~GUI();
};

#endif // GUI_HPP_INCLUDED
